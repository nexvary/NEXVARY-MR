package com.nexvary.npse.core;
/** Capability to attempt a probe is distinct from a successful observation. */
public final class AccessPolicy {
    private AccessPolicy() {}
    public enum State { UNAVAILABLE, PERMISSION_REQUIRED, DENIED, SHELL, UNSUPPORTED_ROOT }
    public static State evaluate(boolean alive, boolean granted, int uid, boolean denied) {
        if (!alive) return State.UNAVAILABLE;
        if (uid == 0) return State.UNSUPPORTED_ROOT;
        if (uid != 2000) return State.UNAVAILABLE;
        if (granted) return State.SHELL;
        return denied ? State.DENIED : State.PERMISSION_REQUIRED;
    }
    public static boolean callerAllowed(int callerUid, int ownerUid) {
        return ownerUid >= 10000 && callerUid == ownerUid;
    }
}
