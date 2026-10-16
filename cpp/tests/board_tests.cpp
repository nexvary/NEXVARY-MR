#include "BoardBridge.hpp"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>
#include <QUrl>
#include <limits>

class BoardTests final : public QObject {
  Q_OBJECT
private:
  QTemporaryDir dir;
  static QJsonObject fixture() {
    return {{"format", "nexvary.board.v1"}, {"title", "Synthetic test board"}, {"author", "NEXVARY test"}, {"license", "CC0-1.0"}, {"width", 100}, {"height", 80},
            {"nets", QJsonArray{QJsonObject{{"name", "DEMO_V"}, {"referenceVoltage", 3.3}, {"tolerance", 0.1}}, QJsonObject{{"name", "DEMO_S"}}}},
            {"components", QJsonArray{QJsonObject{{"reference", "U1"}, {"name", "Fictional controller"}, {"x", 10}, {"y", 20}, {"width", 30}, {"height", 40}, {"nets", QJsonArray{"DEMO_V", "DEMO_S"}}}}}};
  }
  QString write(const QJsonObject &object, const QString &name = "board.json") {
    const auto path = dir.filePath(name); QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) return {};
    if (file.write(QJsonDocument(object).toJson()) < 0) return {};
    return path;
  }
private slots:
  void importAndSearch() {
    BoardBridge board; QSignalSpy changed(&board, &BoardBridge::boardChanged);
    const auto path = write(fixture()); QVERIFY(!path.isEmpty());
    QVERIFY(!board.importBoard(path)); QVERIFY(!board.loaded());
    QVERIFY(board.importBoard(QUrl::fromLocalFile(path).toString(), true));
    QCOMPARE(changed.count(), 1); QVERIFY(board.loaded()); QCOMPARE(board.boardWidth(), 100.0);
    QCOMPARE(board.search("controller").size(), 1); QCOMPARE(board.search("demo_v").size(), 1);
    QCOMPARE(board.search("U1").size(), 1); QCOMPARE(board.search("").size(), 1); QVERIFY(board.search("missing").isEmpty());
    QCOMPARE(board.author(), QString("NEXVARY test")); QCOMPARE(board.license(), QString("CC0-1.0"));
  }
  void malformedImportKeepsCurrentBoard() {
    BoardBridge board; QVERIFY(board.importBoard(write(fixture()), true));
    QVERIFY(board.addMeasurement("DEMO_V", 3.3));
    auto bad = fixture(); bad.remove("license");
    QVERIFY(!board.importBoard(write(bad, "bad.json"), true)); QCOMPARE(board.title(), QString("Synthetic test board")); QCOMPARE(board.measurements().size(), 1);
    bad = fixture(); auto cs = bad["components"].toArray(); auto c = cs[0].toObject(); c["x"] = 99; cs[0] = c; bad["components"] = cs;
    QVERIFY(!board.importBoard(write(bad, "bounds.json"), true));
    bad = fixture(); cs = bad["components"].toArray(); cs.append(cs[0]); bad["components"] = cs;
    QVERIFY(!board.importBoard(write(bad, "duplicate.json"), true));
    bad = fixture(); auto ns = bad["nets"].toArray(); auto n = ns[0].toObject(); n["tolerance"] = -1; ns[0] = n; bad["nets"] = ns;
    QVERIFY(!board.importBoard(write(bad, "tolerance.json"), true));
    bad = fixture(); cs = bad["components"].toArray(); c = cs[0].toObject(); c["nets"] = QJsonArray{"UNKNOWN"}; cs[0] = c; bad["components"] = cs;
    QVERIFY(!board.importBoard(write(bad, "net.json"), true));
    QVERIFY(!board.importBoard("https://example.invalid/board.json", true));
    QVERIFY(!board.importBoard(dir.filePath("absent.json"), true));
  }
  void measurementsAndAnnotations() {
    BoardBridge board; QVERIFY(board.importBoard(write(fixture()), true));
    QVERIFY(board.addMeasurement("DEMO_V", 3.4, "manual"));
    QCOMPARE(board.measurements().last().toMap()["comparison"].toString(), QString("within_reference"));
    QVERIFY(board.addMeasurement("DEMO_V", 4));
    QCOMPARE(board.measurements().last().toMap()["comparison"].toString(), QString("outside_reference"));
    QVERIFY(board.addMeasurement("DEMO_S", -1));
    QCOMPARE(board.measurements().last().toMap()["comparison"].toString(), QString("no_reference"));
    QVERIFY(!board.addMeasurement("DEMO_V", std::numeric_limits<double>::infinity()));
    QVERIFY(!board.addMeasurement("DEMO_V", std::numeric_limits<double>::quiet_NaN()));
    QVERIFY(!board.addMeasurement("UNKNOWN", 3)); QVERIFY(!board.addMeasurement("DEMO_V", 1001));
    QVERIFY(board.addAnnotation("U1", QString::fromUtf8("قياس يدوي"), 20, 30));
    QVERIFY(!board.addAnnotation("U1", "", 20, 30)); QVERIFY(!board.addAnnotation("U2", "note", 20, 30));
    QVERIFY(!board.addAnnotation("U1", "note", -1, 30));
    QVERIFY(!board.addAnnotation("U1", "note", std::numeric_limits<double>::quiet_NaN(), 30));
  }
  void sessionRoundTripAndTamper() {
    BoardBridge board; const auto source = write(fixture()); QVERIFY(board.importBoard(source, true));
    QVERIFY(board.addMeasurement("DEMO_V", 3.3, "manual input")); QVERIFY(board.addAnnotation("U1", "inspect", 1, 2));
    QVERIFY(board.dirty());
    const auto session = dir.filePath("work.session.json");
    QVERIFY(!board.saveSession(source)); QVERIFY(!board.saveSession("https://example.invalid/session.json"));
    QVERIFY(board.saveSession(QUrl::fromLocalFile(session).toString()));
    QVERIFY(!board.dirty());
    BoardBridge copy; QVERIFY(copy.importBoard(source, true)); QVERIFY(copy.loadSession(session));
    QCOMPARE(copy.measurements(), board.measurements()); QCOMPARE(copy.annotations(), board.annotations());
    QVERIFY(!copy.dirty());
    QFile file(session); QVERIFY(file.open(QIODevice::ReadOnly)); auto root = QJsonDocument::fromJson(file.readAll()).object(); file.close();
    auto measures = root["measurements"].toArray(); auto measurement = measures[0].toObject(); measurement["comparison"] = "outside_reference"; measures[0] = measurement; root["measurements"] = measures;
    QVERIFY(copy.loadSession(write(root, "tampered-comparison.json")));
    QCOMPARE(copy.measurements()[0].toMap()["comparison"].toString(), QString("within_reference"));
    auto invalid = root; auto badNotes = invalid["annotations"].toArray(); auto note = badNotes[0].toObject(); note["x"] = 200; badNotes[0] = note; invalid["annotations"] = badNotes;
    QVERIFY(!copy.loadSession(write(invalid, "bad-position.json"))); QCOMPARE(copy.annotations().size(), 1);
    root["boardSha256"] = "invalid"; QVERIFY(!copy.loadSession(write(root, "wrong-board.json"))); QCOMPARE(copy.measurements().size(), 1);
    auto changed = fixture(); changed["title"] = "Different board";
    QVERIFY(copy.importBoard(write(changed, "different.json"), true)); QVERIFY(!copy.loadSession(session)); QVERIFY(copy.measurements().isEmpty());
  }
  void rejectsLargeFiles() {
    QFile file(dir.filePath("large.json")); QVERIFY(file.open(QIODevice::WriteOnly)); QVERIFY(file.resize(8 * 1024 * 1024 + 1)); file.close();
    BoardBridge board; QVERIFY(!board.importBoard(file.fileName(), true)); QVERIFY(board.error().contains("8 MiB"));
  }
};
QTEST_GUILESS_MAIN(BoardTests)
#include "board_tests.moc"
