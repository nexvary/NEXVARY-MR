#include "PolicyAudit.hpp"
#include "CaseVault.hpp"
#include "IdentityService.hpp"
#include "RecoveryFile.hpp"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QStringDecoder>
#include <QUuid>
#include <sodium.h>
namespace {
constexpr qsizetype maximum = 16 * 1024 * 1024;

constexpr qsizetype eventLimit = 8192;

const QByteArray domain("NEXVARY-MR/policy-audit/v1\n");

const QByteArray actorDomain("NEXVARY-MR/local-attestation/v1\0", 32);

QByteArray json(const QVariantMap &v) {
  return QJsonDocument(QJsonObject::fromVariantMap(v))
      .toJson(QJsonDocument::Compact);
}
QString hash(const QByteArray &v) {
  return QString::fromLatin1(
      QCryptographicHash::hash(v, QCryptographicHash::Sha256).toHex());
}
QString b64(const QByteArray &v) { return QString::fromLatin1(v.toBase64()); }
QByteArray un64(const QVariant &v) {
  return QByteArray::fromBase64(v.toString().toLatin1(),
                                QByteArray::AbortOnBase64DecodingErrors);
}
bool signature(const QByteArray &bytes, const QByteArray &sig,
               const QByteArray &pk) {
  return sig.size() == crypto_sign_BYTES &&
         pk.size() == crypto_sign_PUBLICKEYBYTES &&
         crypto_sign_verify_detached(
             reinterpret_cast<const unsigned char *>(sig.constData()),
             reinterpret_cast<const unsigned char *>(bytes.constData()),
             static_cast<unsigned long long>(bytes.size()),
             reinterpret_cast<const unsigned char *>(pk.constData())) == 0;
}
bool uniqueKeys(const QByteArray &bytes) {
  QList<QSet<QString>> stack;
  for (qsizetype i = 0; i < bytes.size(); ++i) {
    const auto c = bytes[i];
    if (c == '{' || c == '[') {
      if (stack.size() >= 32)
        return false;
      stack.append(QSet<QString>());
    } else if (c == '}' || c == ']') {
      if (stack.isEmpty())
        return false;
      stack.removeLast();
    } else if (c == '"') {
      const auto start = i++;
      bool ended = false;
      for (; i < bytes.size(); ++i) {
        if (bytes[i] == '\\') {
          ++i;
          continue;
        }
        if (bytes[i] == '"') {
          ended = true;
          break;
        }
      }
      if (!ended)
        return false;
      auto next = i + 1;
      while (next < bytes.size() && QByteArray(" \t\r\n").contains(bytes[next]))
        ++next;
      if (next < bytes.size() && bytes[next] == ':') {
        if (stack.isEmpty())
          return false;
        const auto decoded = QJsonDocument::fromJson(
            "[" + bytes.mid(start, i - start + 1) + "]");
        if (!decoded.isArray() || decoded.array().size() != 1 ||
            !decoded.array()[0].isString())
          return false;
        const auto name = decoded.array()[0].toString();
        if (stack.last().contains(name))
          return false;
        stack.last().insert(name);
      }
    }
  }
  return stack.isEmpty();
}
QVariantMap decode(const QByteArray &bytes) {
  QStringDecoder utf8(QStringDecoder::Utf8);
  // QString forces the converter to run before hasError(); auto is lazy in Qt.
  const QString decoded = utf8(bytes);
  Q_UNUSED(decoded);
  if (utf8.hasError())
    return {};
  if (!uniqueKeys(bytes))
    return {};
  QJsonParseError e;
  const auto d = QJsonDocument::fromJson(bytes, &e);
  return e.error == QJsonParseError::NoError && d.isObject()
             ? d.object().toVariantMap()
             : QVariantMap();
}
QByteArray load(const QString &path) {
  QFile f(path);
  if (QFileInfo(path).isSymLink() || !mr::recovery::openRegularInput(f) ||
      f.size() > maximum + 1024 * 1024)
    return {};
  return f.readAll();
}
} // namespace
PolicyAudit::PolicyAudit(const QString &path, bool synthetic)
    : path_(mr::recovery::localFilePath(path)), synthetic_(synthetic) {
  if (sodium_init() < 0 || path_.isEmpty() ||
      (!synthetic_ && !mr::recovery::vaultAvailable())) {
    fail("Protected policy audit unavailable; all exceptions denied.");
    return;
  }
  if (!QDir().mkpath(QFileInfo(path_).absolutePath())) {
    fail("Cannot create policy audit directory.");
    return;
  }
  QLockFile lock(path_ + ".lock");
  if (!lock.tryLock(0)) {
    fail("Policy audit busy; restart after the other writer closes.");
    return;
  }
  if (QFileInfo::exists(path_)) {
    const auto raw = load(path_);
    QString error;
    auto plain = mr::recovery::openCaseBytes(raw, synthetic_, error);
    const auto state = decode(plain);
    sodium_memzero(plain.data(), static_cast<size_t>(plain.size()));

    journalId_ = state.value("journalId").toString();
    publicKey_ = state.value("publicKey").toString();
    secret_ = un64(state.value("secretKey"));
    events_ = state.value("events").toList();

    unsigned char pk[crypto_sign_PUBLICKEYBYTES]{};

    if (secret_.size() != crypto_sign_SECRETKEYBYTES) {
      fail("Policy audit key cannot be opened.");
      return;
    }
    crypto_sign_ed25519_sk_to_pk(
        pk, reinterpret_cast<const unsigned char *>(secret_.constData()));

    if (b64(QByteArray(reinterpret_cast<const char *>(pk), sizeof pk)) !=
            publicKey_ ||
        !verifyExport(exportBytes(), publicKey_, {}, error)) {
      fail("Policy audit integrity failed: " + error);
      return;
    }
    fileDigest_ = QCryptographicHash::hash(raw, QCryptographicHash::Sha256);
    healthy_ = true;
    return;
  }
  unsigned char pk[crypto_sign_PUBLICKEYBYTES], sk[crypto_sign_SECRETKEYBYTES];
  crypto_sign_keypair(pk, sk);
  publicKey_ = b64(QByteArray(reinterpret_cast<const char *>(pk), sizeof pk));
  secret_ = QByteArray(reinterpret_cast<const char *>(sk), sizeof sk);
  sodium_memzero(sk, sizeof sk);
  journalId_ = QUuid::createUuid().toString(QUuid::WithoutBraces);

  healthy_ = save({{"schema", "nexvary-policy-audit/v1"},
                   {"journalId", journalId_},
                   {"publicKey", publicKey_},
                   {"secretKey", b64(secret_)},
                   {"events", events_}});
}
PolicyAudit::~PolicyAudit() {
  sodium_memzero(secret_.data(), static_cast<size_t>(secret_.size()));
}
bool PolicyAudit::fail(const QString &message) {
  healthy_ = false;
  error_ = message;
  return false;
}
bool PolicyAudit::save(const QVariantMap &state) {
  auto plain = json(state);
  if (plain.size() > maximum)
    return fail("Policy audit capacity reached; retain/export it and arrange "
                "reviewed rotation. Exceptions denied.");

  QString error;
  const auto sealed = mr::recovery::sealCase(plain, synthetic_, error);
  sodium_memzero(plain.data(), static_cast<size_t>(plain.size()));

  QSaveFile file(path_);
  file.setDirectWriteFallback(false);

  if (sealed.isEmpty() || QFileInfo(path_).isSymLink() ||
      !file.open(QIODevice::WriteOnly) ||
      !file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner) ||
      file.write(sealed) != sealed.size() || !file.commit())
    return fail("Cannot commit protected policy audit; exceptions denied.");

  fileDigest_ = QCryptographicHash::hash(sealed, QCryptographicHash::Sha256);
  return true;
}
bool PolicyAudit::check() {
  if (!healthy_)
    return false;
  const auto raw = load(path_);
  if (raw.isEmpty() ||
      QCryptographicHash::hash(raw, QCryptographicHash::Sha256) != fileDigest_)
    return fail(
        "Policy audit changed/disappeared outside this writer; exceptions "
        "denied. Inspect and restart, never silently repair.");
  return true;
}
bool PolicyAudit::append(const QString &action, const QVariantMap &record,
                         const QString &result, IdentityService *actor) {
  if (!check())
    return false;

  QLockFile lock(path_ + ".lock");
  if (!lock.tryLock(0))
    return fail("Policy audit is busy; exceptions denied.");

  if (!check())
    return false;

  if (events_.size() >= eventLimit)
    return fail("Policy audit event capacity reached; exceptions denied.");

  QVariantMap body{
      {"action", action},
      {"record", record},
      {"result", result},
      {"timeUtc", QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs)},
      {"principalId", actor ? actor->userId() : QString()},
      {"principalRole", actor ? actor->session().value("role")
                              : QVariant("system-observation")}};

  QVariantMap proof;

  if (actor) {
    auto e = actor->attest("government-policy-event", body);
    if (e.isEmpty())
      return fail("Policy actor signature failed; exceptions denied.");

    const auto sig = e.take("signature");
    proof = {{"message", b64(actorDomain + json(e))},
             {"signature", sig},
             {"publicKey", e.value("publicKey")}};
  }
  QVariantMap payload = body;
  payload.insert("journalId", journalId_);
  payload.insert("sequence", events_.size() + 1);
  payload.insert("previous",
                 events_.isEmpty()
                     ? QString(64, '0')
                     : events_.last().toMap().value("hash").toString());
  payload.insert("actorProof", proof);

  const auto bytes = json(payload);
  unsigned char sig[crypto_sign_BYTES];
  crypto_sign_detached(
      sig, nullptr,
      reinterpret_cast<const unsigned char *>((domain + bytes).constData()),
      static_cast<unsigned long long>(domain.size() + bytes.size()),
      reinterpret_cast<const unsigned char *>(secret_.constData()));
  const QByteArray signedBytes(reinterpret_cast<const char *>(sig), sizeof sig);

  auto next = events_;
  next.append(QVariantMap{{"payload", b64(bytes)},
                          {"signature", b64(signedBytes)},
                          {"hash", hash(bytes + signedBytes)}});

  if (!save({{"schema", "nexvary-policy-audit/v1"},
             {"journalId", journalId_},
             {"publicKey", publicKey_},
             {"secretKey", b64(secret_)},
             {"events", next}}))
    return false;

  events_ = next;
  return true;
}
QVariantMap PolicyAudit::anchor() const {
  return {{"journalId", journalId_},
          {"publicKey", publicKey_},
          {"sequence", events_.size()},
          {"hash", events_.isEmpty() ? QString(64, '0')
                                     : events_.last().toMap().value("hash")}};
}
QByteArray PolicyAudit::exportBytes() const {
  return json({{"schema", "nexvary-policy-audit/v1"},
               {"journalId", journalId_},
               {"publicKey", publicKey_},
               {"events", events_}});
}
QVariantMap PolicyAudit::parseAnchor(const QByteArray &bytes) {
  if (bytes.size() > 4096)
    return {};
  const auto a = decode(bytes);
  if (a.keys() != QStringList{"hash", "journalId", "publicKey", "sequence"})
    return {};
  return a;
}
bool PolicyAudit::verifyExport(const QByteArray &bytes,
                               const QString &expectedKey,
                               const QVariantMap &retained, QString &error) {
  error.clear();
  if (sodium_init() < 0) {
    error = "Signature verifier unavailable.";
    return false;
  }
  const auto reject = [&error](const QString &why) {
    error = why;
    return false;
  };
  const auto doc = decode(bytes);
  const auto pk = un64(expectedKey);
  const auto rows = doc.value("events").toList();

  if (bytes.isEmpty() || bytes.size() > maximum ||
      doc.value("schema") != "nexvary-policy-audit/v1" ||
      doc.value("publicKey") != expectedKey ||
      pk.size() != crypto_sign_PUBLICKEYBYTES ||
      QUuid(doc.value("journalId").toString()).isNull() ||
      doc.value("events").metaType().id() != QMetaType::QVariantList ||
      rows.size() > eventLimit)
    return reject("Audit format/key/size rejected.");

  QString previous(64, '0');
  int sequence = 0;
  QMap<QString, QVariantMap> states;
  QMap<QString, QString> principalKeys;

  if (!retained.isEmpty() &&
      (retained.value("journalId") != doc.value("journalId") ||
       retained.value("publicKey") != expectedKey ||
       retained.value("sequence").metaType().id() == QMetaType::Bool ||
       retained.value("sequence").toDouble() !=
           retained.value("sequence").toLongLong() ||
       !QRegularExpression("^[0-9a-f]{64}$")
            .match(retained.value("hash").toString())
            .hasMatch() ||
       retained.value("sequence").toLongLong() < 0 ||
       retained.value("sequence").toLongLong() > rows.size()))
    return reject("Retained anchor does not match this history.");

  if (!retained.isEmpty() && retained.value("sequence").toInt() == 0 &&
      retained.value("hash") != previous)
    return reject("Empty anchor mismatch.");

  for (const auto &v : rows) {
    const auto e = v.toMap();
    const auto raw = un64(e.value("payload")), sig = un64(e.value("signature"));
    auto p = decode(raw);

    if (raw.isEmpty() || raw.size() > 32 * 1024 ||
        !signature(domain + raw, sig, pk) ||
        e.value("hash") != hash(raw + sig) ||
        p.value("sequence").toLongLong() != ++sequence ||
        p.value("previous") != previous ||
        p.value("journalId") != doc.value("journalId"))
      return reject("Event signature, sequence or hash link rejected.");

    const auto proof = p.take("actorProof").toMap();
    p.remove("sequence");
    p.remove("previous");
    p.remove("journalId");

    if (!proof.isEmpty()) {
      const auto message = un64(proof.value("message"));
      const auto a = decode(message.mid(actorDomain.size()));

      if (!message.startsWith(actorDomain) ||
          !signature(message, un64(proof.value("signature")),
                     un64(proof.value("publicKey"))) ||
          a.value("schema").toInt() != 1 ||
          a.value("purpose") != "government-policy-event" ||
          a.value("payload").toMap() != p ||
          a.value("signerId") != p.value("principalId") ||
          a.value("role") != p.value("principalRole") ||
          a.value("publicKey") != proof.value("publicKey"))
        return reject("Actor attestation rejected.");
      const auto signer = a.value("signerId").toString(),
                 signerKey = proof.value("publicKey").toString();
      if (QUuid(signer).isNull() || (principalKeys.contains(signer) &&
                                     principalKeys.value(signer) != signerKey))
        return reject("Principal key replacement rejected.");
      principalKeys.insert(signer, signerKey);

    } else if (!p.value("principalId").toString().isEmpty() ||
               p.value("principalRole") != "system-observation")
      return reject("Missing actor attestation.");

    const auto action = p.value("action").toString(),
               role = p.value("principalRole").toString(),
               actor = p.value("principalId").toString();
    const auto r = p.value("record").toMap();
    const auto id = r.value("id").toString();

    if (action == "request") {
      if (QUuid(id).isNull() || states.contains(id) ||
          r.value("requester") != actor ||
          (role != "administrator" && role != "investigator" &&
           role != "repair-technician") ||
          r.value("status") != "requested" ||
          r.value("requesterRole") != role ||
          !QStringList{"cloud-publish", "tracking-portal", "signature-update",
                       "device-tools", "external-analysis", "external-links"}
               .contains(r.value("capability").toString()) ||
          r.value("durationMs").toLongLong() < 1000 ||
          r.value("durationMs").toLongLong() > 600000 ||
          r.value("scope") !=
              "category-and-requester; not destination containment" ||
          (!r.value("caseRef").toString().isEmpty() &&
           QUuid(r.value("caseRef").toString()).isNull()))
        return reject("Request authority/state rejected.");
      states.insert(id, r);

    } else if (action == "review" || action == "approve" ||
               action == "reject" || action == "activate" ||
               action == "revoke" || action == "expire" ||
               action == "session-close" || action == "restart-close" ||
               action == "emergency-stop") {
      const auto old = states.value(id);
      auto invariantOld = old, invariantNew = r;
      QStringList mutableKeys{"status"};
      if (action == "review")
        mutableKeys << "reviewer" << "reviewReason";
      else if (action == "approve" || action == "reject")
        mutableKeys << "decisionReason";
      else if (action == "activate")
        mutableKeys << "activatedUtc" << "expiresUtc";
      if (action == "reject" || action == "expire" || action == "revoke" ||
          action == "session-close" || action == "restart-close" ||
          action == "emergency-stop")
        mutableKeys << "closedUtc";
      for (const auto &key : mutableKeys) {
        invariantOld.remove(key);
        invariantNew.remove(key);
      }
      const auto status = old.value("status").toString();
      if (old.isEmpty() || invariantOld != invariantNew)
        return reject("Exception identity/scope mutation rejected.");

      if (action == "review" &&
          (status != "requested" ||
           (role != "administrator" && role != "reviewer") ||
           actor == old.value("requester") || r.value("reviewer") != actor ||
           r.value("status") != "in-review"))
        return reject("Review separation rejected.");

      if ((action == "approve" || action == "reject") &&
          (status != "in-review" || actor != old.value("reviewer") ||
           actor == old.value("requester") ||
           (role != "administrator" && role != "reviewer") ||
           r.value("reviewer") != old.value("reviewer") ||
           r.value("status") !=
               (action == "approve" ? "approved" : "rejected")))
        return reject("Decision authority/state rejected.");

      if (action == "activate" &&
          (status != "approved" || actor != old.value("requester") ||
           r.value("reviewer") != old.value("reviewer") ||
           r.value("status") != "active"))
        return reject("Activation authority/state rejected.");

      if ((action == "expire" || action == "session-close" ||
           action == "restart-close" || action == "emergency-stop" ||
           action == "revoke") &&
          (status == "rejected" || status == "revoked" || status == "expired" ||
           r.value("status") != (action == "expire" ? "expired" : "revoked")))
        return reject("Closure state rejected.");

      if (action == "revoke" && role != "system-observation" &&
          role != "administrator" && actor != old.value("requester") &&
          actor != old.value("reviewer"))
        return reject("Revocation authority rejected.");
      states.insert(id, r);

    } else if (action != "operation-allowed" && action != "operation-denied" &&
               action != "browser-result" && action != "audit-verification" &&
               action != "policy-rejection")
      return reject("Unknown event action.");

    if (action == "operation-allowed" || action == "browser-result") {
      const auto current = states.value(id);
      if (current.isEmpty() || r != current || r.value("status") != "active" ||
          role != "system-observation")
        return reject("Operation gate has no matching active exception.");
    }
    if ((action == "expire" || action == "session-close" ||
         action == "restart-close" || action == "emergency-stop") &&
        role != "system-observation")
      return reject("System closure attribution rejected.");
    if (action == "audit-verification" && role != "administrator" &&
        role != "reviewer" && role != "auditor")
      return reject("Audit verifier authority rejected.");
    previous = e.value("hash").toString();
    if (!retained.isEmpty() && sequence == retained.value("sequence").toInt() &&
        retained.value("hash") != previous)
      return reject("Retained history prefix mismatch.");
  }
  return true;
}
