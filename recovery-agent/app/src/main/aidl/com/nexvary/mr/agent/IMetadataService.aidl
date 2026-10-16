package com.nexvary.mr.agent;
// No command, pathname, package name, or user argument crosses this interface.
interface IMetadataService {
    String collectMetadata();
    void destroy() = 16777114;
}
