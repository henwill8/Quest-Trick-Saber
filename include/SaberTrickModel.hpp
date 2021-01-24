#pragma once

#include "PluginConfig.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/typedefs-array.hpp"
#include "beatsaber-hook/shared/utils/typedefs.h"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Rigidbody.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/Collider.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/SetSaberGlowColor.hpp"
#include "GlobalNamespace/SetSaberFakeGlowColor.hpp"
#include "GlobalNamespace/SaberTypeObject.hpp"
#include "GlobalNamespace/SaberModelContainer.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "System/Collections/Generic/List_1.hpp"

class SaberTrickModel {
  public:
    UnityEngine::Rigidbody* Rigidbody = nullptr;
    UnityEngine::GameObject* SaberGO;  // GameObject

    SaberTrickModel(UnityEngine::GameObject* SaberModel, bool basicSaber) {
        CRASH_UNLESS(SaberModel);
        getLogger().debug("SaberTrickModel construction!");
        // il2cpp_utils::LogClass(il2cpp_functions::class_from_system_type(tRigidbody), false);

        SaberGO = OriginalSaberModel = SaberModel;

        if (PluginConfig::Instance().EnableTrickCutting) {
            Rigidbody = SaberModel->GetComponent<UnityEngine::Rigidbody*>();
            if (!Rigidbody) {
                getLogger().warning("Adding rigidbody to original SaberModel?!");
                Rigidbody = SaberModel->AddComponent<UnityEngine::Rigidbody*>();
            }
            SetupRigidbody(Rigidbody, OriginalSaberModel);
        } else {
            TrickModel = UnityEngine::Object::Instantiate(SaberModel);
            CRASH_UNLESS(TrickModel);
            FixBasicTrickSaber(TrickModel, basicSaber);
            AddTrickRigidbody();

            auto* str = CRASH_UNLESS(il2cpp_utils::createcsstr("VRGameCore"));
            auto* vrGameCore = UnityEngine::GameObject::Find(str);
            auto* vrGameCoreT = vrGameCore->get_transform();
            auto* trickModelT = TrickModel->get_transform();

            trickModelT->SetParent(vrGameCoreT);
            TrickModel->SetActive(false);
        }
        getLogger().debug("Leaving SaberTrickModel construction!");
    }

    void SetupRigidbody(UnityEngine::Rigidbody* rigidbody, UnityEngine::GameObject* model) {
        rigidbody->set_useGravity(false);
        rigidbody->set_isKinematic(true);

        static auto set_detectCollisions = (function_ptr_t<void, Il2CppObject*, bool>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_detectCollisions"));
        getLogger().debug("set_detectCollisions ptr offset: %lX", asOffset(set_detectCollisions));
        set_detectCollisions(rigidbody, false);

        static auto set_maxAngVel = (function_ptr_t<void, Il2CppObject*, float>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_maxAngularVelocity"));
        getLogger().debug("set_maxAngVel ptr offset: %lX", asOffset(set_maxAngVel));
        set_maxAngVel(rigidbody, 800.0f);

        static auto set_interp = (function_ptr_t<void, Il2CppObject*, int>)CRASH_UNLESS(
            il2cpp_functions::resolve_icall("UnityEngine.Rigidbody::set_interpolation"));
        getLogger().debug("set_interpolation ptr offset: %lX", asOffset(set_interp));
        set_interp(rigidbody, 1);  // Interpolate

        Array<UnityEngine::Collider*>* colliders = model->GetComponentsInChildren<UnityEngine::Collider*>(true); //CRASH_UNLESS(il2cpp_utils::RunMethod<Array<Il2CppObject*>*>(model, "GetComponentsInChildren", tCollider, true));


        for (int i = 0; i < colliders->Length(); i++) {
            colliders->values[i]->set_enabled(false);
        }
    }

    void AddTrickRigidbody() {
        Rigidbody = TrickModel->AddComponent<UnityEngine::Rigidbody*>(); //CRASH_UNLESS(il2cpp_utils::RunMethod(TrickModel, "AddComponent", tRigidbody));
        CRASH_UNLESS(Rigidbody);
        SetupRigidbody(Rigidbody, TrickModel);
    }

    void FixBasicTrickSaber(UnityEngine::GameObject* newSaber, bool basic) {
        if (!basic) return;
        getLogger().debug("Fixing basic trick saber color!");

        GlobalNamespace::SaberModelContainer* saberModelContainer = SaberGO->GetComponentsInParent<GlobalNamespace::SaberModelContainer*>(true)->values[0];

        GlobalNamespace::SaberTypeObject* _saberTypeObject = saberModelContainer->saber->saberType;
        GlobalNamespace::SaberType saberType = _saberTypeObject->saberType; // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(_saberTypeObject, "saberType"));
        getLogger().debug("saber type: %i", saberType);
//        CRASH_UNLESS(saberType);
        auto* saberModelContainerT = saberModelContainer->get_transform();
        getLogger().debug("saber container");
        CRASH_UNLESS(saberModelContainerT);

        auto* saberModelController = newSaber->GetComponent<GlobalNamespace::SaberModelController*>();
        getLogger().debug("saber controller");
        CRASH_UNLESS(saberModelController);

        auto* origModelController = SaberGO->GetComponent<GlobalNamespace::SaberModelController*>(); // CRASH_UNLESS(il2cpp_utils::RunMethod(SaberGO, "GetComponent", tSaberModelController));
        auto* colorMgr = origModelController->colorManager; // CRASH_UNLESS(il2cpp_utils::GetFieldValue(origModelController, "_colorManager"));
        getLogger().debug("saber color manager");
        CRASH_UNLESS(colorMgr);
        saberModelController->colorManager = colorMgr;

        auto* glows = saberModelController->setSaberGlowColors; // CRASH_UNLESS(il2cpp_utils::GetFieldValue<Il2CppArray*>(saberModelController, "_setSaberGlowColors"));
        getLogger().debug("_setSaberGlowColors.length: %u", glows->Length());
        for (int i = 0; i < glows->Length(); i++) {
            GlobalNamespace::SetSaberGlowColor* obj = glows->values[i];

            obj->colorManager = colorMgr;
        }

        auto* fakeGlows = saberModelController->setSaberFakeGlowColors;
        getLogger().debug("_setSaberFakeGlowColors.length: %u", fakeGlows->Length());
        for (int i = 0; i < fakeGlows->Length(); i++) {
            GlobalNamespace::SetSaberFakeGlowColor* obj = fakeGlows->values[i];

            obj->colorManager = colorMgr;
        }

        saberModelController->Init(saberModelContainerT, saberModelContainer->saber);
    }

    void ChangeToTrickModel() {
        if (PluginConfig::Instance().EnableTrickCutting) return;
        TrickModel->SetActive(true);

        auto* trickT = TrickModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(TrickModel, "transform"));
        auto* origT = OriginalSaberModel->get_transform(); // CRASH_UNLESS(il2cpp_utils::GetPropertyValue(OriginalSaberModel, "transform"));
        auto pos = origT->get_position(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Vector3>(origT, "position"));
        auto rot = origT->get_rotation(); //CRASH_UNLESS(il2cpp_utils::GetPropertyValue<Quaternion>(origT, "rotation"));

        trickT->set_position(pos);
        trickT->set_rotation(rot);
        OriginalSaberModel->SetActive(false);

        SaberGO = TrickModel;
    }

    void ChangeToActualSaber() {
        if (PluginConfig::Instance().EnableTrickCutting) return;

        OriginalSaberModel->SetActive(true);
        TrickModel->SetActive(false);
        SaberGO = OriginalSaberModel;
    }


  private:
    UnityEngine::GameObject* OriginalSaberModel = nullptr;  // GameObject
    UnityEngine::GameObject* TrickModel = nullptr;          // GameObject
};
