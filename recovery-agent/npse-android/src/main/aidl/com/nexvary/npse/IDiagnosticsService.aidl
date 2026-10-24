package com.nexvary.npse;
// No user-supplied commands, package names or paths cross this interface.
interface IDiagnosticsService {
    String collectDiagnostics() = 0;
    void cancel() = 1;
    void destroy() = 16777114;
}
