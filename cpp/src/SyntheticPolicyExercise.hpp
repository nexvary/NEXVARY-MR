#pragma once
#include "GovernmentOperationPolicy.hpp"
#include "IdentityService.hpp"
// Explicit synthetic test/deployed-smoke exercise. This is the real review
// protocol, not a permission bypass or a provisioned production credential.
inline bool performSyntheticReviewedException(IdentityService &identity,
                                              GovernmentOperationPolicy &policy,
                                              const QString &category,
                                              const QString &reason,
                                              const QString &password) {
  if (!identity.syntheticProfile() ||
      identity.session().value("role") != "administrator")
    return false;
  const auto username = identity.session().value("username").toString();
  bool found = false;
  for (const auto &v : identity.users())
    if (v.toMap().value("username") == "synthetic-policy-reviewer")
      found = true;
  if (!found &&
      identity
          .createUser("synthetic-policy-reviewer", "Synthetic policy reviewer",
                      "reviewer", password)
          .isEmpty())
    return false;
  const auto id =
      policy.requestException(category, reason, QString(), 600, true);
  if (id.isEmpty() || !identity.login("synthetic-policy-reviewer", password) ||
      !policy.reviewException(id, "Synthetic independent-account review; not "
                                  "proof of independent people") ||
      !policy.decideException(id, true, "Synthetic fixture decision") ||
      !identity.login(username, password))
    return false;
  return policy.activateException(id);
}
