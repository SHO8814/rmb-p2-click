#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EditorPauseLayer.hpp>

using namespace geode::prelude;

// Default state (prevents input bug)
static bool sendingPlayer2Input = false;

// Returns saved RMB button state
bool rmbPlayer2Enabled() {
    return Mod::get()->getSavedValue<bool>("rmb-enabled", true);
}

// Creates RMB toggle button
CCMenuItemToggler* createRmbToggle(CCObject* target, SEL_MenuHandler callback, float scale)
{
    // button textures
    auto spriteOn = CCSprite::create("rmbButtonOn.png"_spr);
    auto spriteOff = CCSprite::create("rmbButtonOff.png"_spr);

    spriteOn->setScale(scale);
    spriteOff->setScale(scale);

    return CCMenuItemToggler::create(spriteOff, spriteOn, target, callback);
}

// Handle RMB input in-game
class $modify(RMBPlayer2GameLayer, GJBaseGameLayer) {
    void handleButton(bool down, int button, bool isPlayer1) {
        if (!sendingPlayer2Input && button == 1) {
            isPlayer1 = true;
        }

        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }
};

// Listen for RMB input
$execute{
    MouseInputEvent().listen([](MouseInputData& event) {
        if (event.button != MouseInputData::Button::Right) {
            return ListenerResult::Propagate;
        }

        if (!rmbPlayer2Enabled()) {
            return ListenerResult::Propagate;
        }

        auto gameLayer = GJBaseGameLayer::get();

        if (!gameLayer) {
            return ListenerResult::Propagate;
        }

        bool rmbPressed = event.action == MouseInputData::Action::Press;

        sendingPlayer2Input = true;

        gameLayer->handleButton(rmbPressed, 1, false);

        sendingPlayer2Input = false;

        return ListenerResult::Stop;
    }).leak();
}

// Toggle Button in Game Pause
class $modify(RMBPauseLayer, PauseLayer) {
    void customSetup() {
        PauseLayer::customSetup();

        auto gameMenu = this->getChildByID("right-button-menu");

        if (!gameMenu) {
            return;
        }

        auto rmbToggle = createRmbToggle(this, menu_selector(RMBPauseLayer::onRmbToggle), 0.69f); // button size

        rmbToggle->setID("rmb-player2-toggle"_spr);
        rmbToggle->toggle(rmbPlayer2Enabled());

        gameMenu->addChild(rmbToggle);
        gameMenu->reorderChild(rmbToggle, 5); // button priority
        gameMenu->updateLayout();
    }

    void onRmbToggle(CCObject* sender) {
        auto rmbToggle = static_cast<CCMenuItemToggler*>(sender);

        bool isEnabled = !rmbToggle->isToggled();

        Mod::get()->setSavedValue<bool>("rmb-enabled", isEnabled);
    }
};

// Toggle Button in Editor Pause
class $modify(RMBEditorPauseLayer, EditorPauseLayer) {
    bool init(LevelEditorLayer* layer) {
        if (!EditorPauseLayer::init(layer)) {
            return false;
        }

        auto editorMenu = this->getChildByID("guidelines-menu");
        
        if (!editorMenu) {
            return true;
        }

        auto rmbToggle = createRmbToggle(this, menu_selector(RMBEditorPauseLayer::onRmbToggle), 0.88f); // button size

        rmbToggle->toggle(rmbPlayer2Enabled());
        rmbToggle->setID("rmb-player2-toggle"_spr);

        editorMenu->addChild(rmbToggle);
        editorMenu->reorderChild(rmbToggle, 5); // button priority
        editorMenu->updateLayout();

        return true;
    }

    void onRmbToggle(CCObject* sender) {
        auto rmbToggle = static_cast<CCMenuItemToggler*>(sender);

        bool isEnabled = !rmbToggle->isToggled();

        Mod::get()->setSavedValue<bool>("rmb-enabled", isEnabled);
    }
};