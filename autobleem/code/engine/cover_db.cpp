//
// Created by screemer on 2/2/19.
//

#include "cover_db.h"

#include "../environment.h"
#include "../log.h"
#include "../utils/file_utils.h"

Coverdb::Coverdb() {
    const std::string path = Env::getPathToRetroarchRdbDir() + sep + "Sony - PlayStation.rdb";
    if (!FileUtils::exists(path)) {
        PLOG_DEBUG << "rdb file " << path << " not found";
        return;
    }
    if (!reader.open(path)) {
        PLOG_ERROR << "failed to open rdb " << path;
    }
}
