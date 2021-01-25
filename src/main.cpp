#include "../include/main.hpp"
#include "../include/PluginConfig.hpp"
#include "../include/TrickManager.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "UnityEngine/Resources.hpp"
#include "UnityEngine/Object.hpp"
#include "GlobalNamespace/PlayerVRControllersManager.hpp"
#include "GlobalNamespace/GameSongController.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberTypeObject.hpp"
#include "VRUIControls/VRPointer.hpp"
#include "GlobalNamespace/VRController.hpp"
#include "UnityEngine/XR/XRNode.hpp"
#include "UnityEngine/SceneManagement/SceneManager.hpp"
#include "UnityEngine/SceneManagement/LoadSceneMode.hpp"
#include "GlobalNamespace/GamePause.hpp"
#include "GlobalNamespace/OculusVRHelper.hpp"
#include "GlobalNamespace/SaberBurnMarkArea.hpp"
#include "GlobalNamespace/SaberBurnMarkSparkles.hpp"
#include "GlobalNamespace/ObstacleSaberSparkleEffectManager.hpp"
#include "custom-types/shared/register.hpp"

#include "GlobalNamespace/GameScenesManager.hpp"
#include "System/Action.hpp"
#include "System/Action_1.hpp"
#include "Zenject/DiContainer.hpp"
#include "GlobalNamespace/ScenesTransitionSetupDataSO.hpp"

using namespace GlobalNamespace;

// Returns a logger, useful for printing debug messages
Logger& getLogger() {
    static auto* logger = new Logger(modInfo);
    return *logger;
}

extern "C" void setup(ModInfo& info) {
    info.id      = "TrickSaber";
    info.version = "0.2.2";
    modInfo      = info;
    getLogger().info("Leaving setup!");
}

Saber* FakeSaber = nullptr;
Saber* RealSaber = nullptr;
TrickManager leftSaber;
TrickManager rightSaber;

MAKE_HOOK_OFFSETLESS(SceneManager_Internal_SceneLoaded, void, UnityEngine::SceneManagement::Scene scene, UnityEngine::SceneManagement::LoadSceneMode mode) {
    getLogger().info("SceneManager_Internal_SceneLoaded");
    if (auto* nameOpt = scene.get_name()) {
        auto* name = nameOpt;
        auto str = to_utf8(csstrtostr(name));
        getLogger().debug("Scene name internal: %s", str.c_str());
    }
    FakeSaber = nullptr;
    RealSaber = nullptr;
    TrickManager::StaticClear();
    leftSaber.Clear();
    rightSaber.Clear();
    SceneManager_Internal_SceneLoaded(scene, mode);
}

MAKE_HOOK_OFFSETLESS(GameScenesManager_PushScenes, void, GlobalNamespace::GameScenesManager* self, GlobalNamespace::ScenesTransitionSetupDataSO* scenesTransitionSetupData,
                     float minDuration, System::Action* afterMinDurationCallback,
                     System::Action_1<Zenject::DiContainer*>* finishCallback) {
    getLogger().debug("GameScenesManager_PushScenes");
    GameScenesManager_PushScenes(self, scenesTransitionSetupData, minDuration, afterMinDurationCallback, finishCallback);
    PluginConfig::Instance().Reload();
    getLogger().debug("Leaving GameScenesManager_PushScenes");
}

MAKE_HOOK_OFFSETLESS(SaberManager_Start, void, SaberManager* self) {
    SaberManager_Start(self);
    saberManager = self;
    getLogger().debug("SaberManager_Start");
}

MAKE_HOOK_OFFSETLESS(Saber_Start, void, Saber* self) {
    getLogger().debug("Saber_Start");
    Saber_Start(self);
    getLogger().debug("Saber_Start original called");
    // saber->sabertypeobject->sabertype
    SaberType saberType = self->saberType->saberType;
    getLogger().debug("SaberType: %i", saberType);

    auto *vrControllers = UnityEngine::Resources::FindObjectsOfTypeAll<VRController*>();


    getLogger().info("VR controllers: %i", vrControllers->Length());

    for (int i = 0; i<vrControllers->Length(); i++) {
        auto* controller = vrControllers->values[i];

    }


    if (saberType == 0) {
        getLogger().debug("Left?");
//        leftSaber.VRController = vrControllersManager->get_node();
        leftSaber.Saber = self;
        leftSaber._isLeftSaber = true;
        leftSaber.other = &rightSaber;
        leftSaber.Start();
    } else {
        getLogger().debug("Right?");
//        rightSaber.VRController = vrControllersManager->rightVRController;
        rightSaber.Saber = self;
        rightSaber.other = &leftSaber;
        rightSaber.Start();
    }
    RealSaber = self;
}

MAKE_HOOK_OFFSETLESS(Saber_ManualUpdate, void, Saber* self) {
    Saber_ManualUpdate(self);
    if (self == leftSaber.Saber) {
        leftSaber.Update();
    } else if (self == rightSaber.Saber) {
        // rightSaber.LogEverything();
        rightSaber.Update();
    }
}

static std::vector<System::Type*> tBurnTypes;


void DisableBurnMarks(int saberType) {
    if (!FakeSaber) {
        static auto* tSaber = csTypeOf(Saber*);
        auto* core = UnityEngine::GameObject::Find(il2cpp_utils::createcsstr("GameCore"));
        FakeSaber = core->AddComponent<Saber*>();

        FakeSaber->set_enabled(false);

        getLogger().info("FakeSaber.isActiveAndEnabled: %i",FakeSaber->get_isActiveAndEnabled());

        auto* saberTypeObj = RealSaber->saberType;
        FakeSaber->saberType = saberTypeObj;
        getLogger().info("FakeSaber SaberType: %i", FakeSaber->saberType);
    }
    for (auto* type : tBurnTypes) {
        auto *components = UnityEngine::Object::FindObjectsOfType(type);
        for (il2cpp_array_size_t i = 0; i < components->Length(); i++) {
            auto *sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(components->values[i], "_sabers"));
            sabers->values[saberType] = FakeSaber;
        }
    }
}

void EnableBurnMarks(int saberType) {
    for (auto *type : tBurnTypes) {
        auto *components = UnityEngine::Object::FindObjectsOfType(type);
        for (int i = 0; i < components->Length(); i++) {
            getLogger().debug("Burn Type: %s", components->values[i]->get_name());
            auto *sabers = CRASH_UNLESS(il2cpp_utils::GetFieldValue<Array<Saber *> *>(components->values[i], "_sabers"));
            sabers->values[saberType] = saberType ? saberManager->rightSaber : saberManager->leftSaber;
        }
    }
}


MAKE_HOOK_OFFSETLESS(FixedUpdate, void, GlobalNamespace::OculusVRHelper* self) {
    FixedUpdate(self);
    TrickManager::StaticFixedUpdate();
}

MAKE_HOOK_OFFSETLESS(Pause, void, GlobalNamespace::GamePause* self) {
    leftSaber.PauseTricks();
    rightSaber.PauseTricks();
    Pause(self);
    TrickManager::StaticPause();
    getLogger().debug("pause: %i", self->pause);
}

MAKE_HOOK_OFFSETLESS(Resume, void, GlobalNamespace::GamePause* self) {
    Resume(self);
    TrickManager::StaticResume();
    leftSaber.ResumeTricks();
    rightSaber.ResumeTricks();
    getLogger().debug("pause: %i", self->pause);
}

MAKE_HOOK_OFFSETLESS(AudioTimeSyncController_Start, void, AudioTimeSyncController* self) {
    AudioTimeSyncController_Start(self);
    audioTimeSyncController = self;
    getLogger().debug("audio time controller: %i");
}

MAKE_HOOK_OFFSETLESS(SaberClashEffect_Start, void, SaberClashEffect* self) {
    SaberClashEffect_Start(self);
    saberClashEffect = self;
    getLogger().debug("saber clash effect: %i", saberClashEffect);
}

MAKE_HOOK_OFFSETLESS(SaberClashEffect_Disable, void, SaberClashEffect* self) {
    SaberClashEffect_Disable(self);
    saberClashEffect = nullptr;
    getLogger().debug("saber clash disable effect: %i", saberClashEffect);
}

MAKE_HOOK_OFFSETLESS(VRController_Update, void, GlobalNamespace::VRController* self) {
    VRController_Update(self);

    if (self->get_node() == UnityEngine::XR::XRNode::LeftHand) {
        leftSaber.VRController = self;
    }

    if (self->get_node() == UnityEngine::XR::XRNode::RightHand) {
        rightSaber.VRController = self;
    }
}

extern "C" void load() {
    PluginConfig::Init();
    // TODO: config menus
    getLogger().info("Installing hooks...");

    INSTALL_HOOK_OFFSETLESS(getLogger(), SceneManager_Internal_SceneLoaded, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "Internal_SceneLoaded", 2));
    INSTALL_HOOK_OFFSETLESS(getLogger(), GameScenesManager_PushScenes, il2cpp_utils::FindMethodUnsafe("", "GameScenesManager", "PushScenes", 4));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Saber_Start, il2cpp_utils::FindMethod("", "Saber", "Start"));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Saber_ManualUpdate, il2cpp_utils::FindMethod("", "Saber", "ManualUpdate"));

    INSTALL_HOOK_OFFSETLESS(getLogger(), FixedUpdate, il2cpp_utils::FindMethod("", "OculusVRHelper", "FixedUpdate"));
    // INSTALL_HOOK_OFFSETLESS(LateUpdate, il2cpp_utils::FindMethod("", "SaberBurnMarkSparkles", "LateUpdate"));

    INSTALL_HOOK_OFFSETLESS(getLogger(), Pause, il2cpp_utils::FindMethod("", "GamePause", "Pause"));
    INSTALL_HOOK_OFFSETLESS(getLogger(), Resume, il2cpp_utils::FindMethod("", "GamePause", "Resume"));

    INSTALL_HOOK_OFFSETLESS(getLogger(), AudioTimeSyncController_Start, il2cpp_utils::FindMethod("", "AudioTimeSyncController", "Start"));
    INSTALL_HOOK_OFFSETLESS(getLogger(), SaberManager_Start, il2cpp_utils::FindMethod("", "SaberManager", "Start"));

    INSTALL_HOOK_OFFSETLESS(getLogger(), SaberClashEffect_Start, il2cpp_utils::FindMethod("", "SaberClashEffect", "Start"));
//    INSTALL_HOOK_OFFSETLESS(getLogger(), SaberClashEffect_Disable, il2cpp_utils::FindMethod("", "SaberClashEffect", "OnDisable"));

    INSTALL_HOOK_OFFSETLESS(getLogger(), VRController_Update, il2cpp_utils::FindMethod("", "VRController", "Update"));

//    INSTALL_HOOK_OFFSETLESS(getLogger(), VRController_Update, il2cpp_utils::FindMethodUnsafe("", "GameSongController", "StartSong", 1));

    tBurnTypes.push_back(csTypeOf(GlobalNamespace::SaberBurnMarkArea*));
    tBurnTypes.push_back(csTypeOf(GlobalNamespace::SaberBurnMarkSparkles*));
    tBurnTypes.push_back(csTypeOf(GlobalNamespace::ObstacleSaberSparkleEffectManager*));

    getLogger().info("Registering custom types");
    custom_types::Register::RegisterType<TrickSaber::TrickSaberTrailData>();
    getLogger().info("Registered types");

    getLogger().info("Installed all hooks!");
}
