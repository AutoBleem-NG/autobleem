//
// Created by screemer on 2/13/19.
//

#pragma once

#include "ps_game.h"
#include "emu_interceptor.h"

//******************
// PcsxInterceptor
//******************
class PcsxInterceptor : public EmuInterceptor {
  public:
    bool execute(PsGamePtr &game, int resumepoint) override;
    void memcardIn(PsGamePtr &game) override;
    void memcardOut(PsGamePtr &game) override;
    void prepareResumePoint(PsGamePtr &game, int pointId) override;
    void saveResumePoint(PsGamePtr &game, int pointId) override;
    void cleanupConfig(PsGamePtr &game);
};
