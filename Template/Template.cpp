#include "pch.h"
using namespace sdk;


CHAMPIONS("TemplateChampion"); // CHAMPIONS({"Draven", "Jinx", "Ezreal"}); for multiple Champions (AIOs)
PLUGIN_NAME("Default");
PLUGIN_TYPE(PluginTypes::CHAMPION);

static std::shared_ptr<BuffInstance> buff; // can be used in myhero->hasBuff(), to get infos from the buff if its alive


/*
* Iss beeing called right before the myhero->Attack() call is processed. So the moment before the mouse is beeing moved to the target
* @param target: the target which is about to get attacked
* @param process: if this is set to false, the attack wont be triggered. It stops the next AA from beeing processed.
*/
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}


/*
* Iss beeing called right after the windup time has been passed. So the AA cant be canceled anymore.
* 
* @param target: the target which has been attacked
* @param do_aa_reset: if this is set to true, the attack timer will get resetted which leads to another Attack call right without delay
*/
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) {
}

/*
* Iss beeing called on any dash only at the moment.
*
* @param target: the target which is dashing
*/
void GapCloser(GameObjectInstance* sender) {
	if (orbwalker->Mode() == OrbwalkModes::NONE)
		return;
}


/*
* Iss beeing called the moment a new missile is spawned
*
* @param missile: the missile  object
*/
void MissileCreate(std::shared_ptr < MissileInstance> missile) {
}


/*
* Iss beeing called the moment a  missile is removed from memory
*
* @param missile: the missile  object
*/
void MissileRemove(std::shared_ptr<MissileInstance> missile) {
}


/*
* Iss beeing called the moment a valid buff is gained by any attackableunit
*
* @param buff: the missile buff object which holds all buff informations
*/
void BuffGain(std::shared_ptr < BuffInstance> buff) {
	if (buff->SourceIndex() == myhero->Index())
		return;
}


/*
* Iss beeing called the moment a buff not valid anymore
*
* @param buff: the missile buff object which holds all buff informations
*/
void BuffLose(std::shared_ptr < BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
		return;
}


/*
* Iss beeing called the moment an object is spawned
*
* @param obj: the   object
*/
void CreateObject(std::shared_ptr<GameObjectInstance> obj) {
}

/*
* Iss beeing called the moment an object is deleted from memory
*
* @param obj: the   object
*/
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {
}

/*
 *	ActiveSpell
 */
void onActiveSpell(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> active_spell) {


}
void Combo() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr)
		return;

	if (settings->combo.useq && myhero->Q()->isReady() && myhero->inRange(target.get(), myhero->AttackRange())) {
		myhero->Q()->Cast();
	}
	else if (settings->combo.usew && myhero->W()->isReady() && myhero->inRange(target.get(), myhero->W()->CastRange())) {
		myhero->W()->Cast(target);
	}
	else if (settings->combo.usee && myhero->E()->isReady() && myhero->inRange(target.get(), myhero->E()->CastRange())) {

		myhero->E()->Cast(target);
	}
	else if (settings->combo.user && myhero->R()->isReady() && myhero->inRange(target.get(), myhero->R()->CastRange())) {
		myhero->R()->Cast(target);
	}
}

void Herass() {
}

void LaneClear() {
}

void LastHit() {
}

void Update() {
	if (orbwalker->Mode() == OrbwalkModes::COMBO) {
		Combo();
	}
	else if (orbwalker->Mode() == OrbwalkModes::HERASS) {
		Herass();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LANECLEAR) {
		LaneClear();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LASTHIT) {
		LastHit();
	}
}

void Draw() {

}

void Menu() {
	auto root_node = menu->CreateSubmenu("Template");

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);


	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);
	sub_herass->AddCheckbox("Use R", &settings->herass.user);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);
	sub_laneclear->AddCheckbox("Use R", &settings->laneclear.user);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);
	sub_lasthit->AddCheckbox("Use R", &settings->lasthit.user);


	root_node->addItems({ sub_combo,sub_herass,sub_laneclear,sub_lasthit });


	menu->addItem(root_node);
}

void onLoad() {
	std::cout << "Template loaded" << std::endl;

	Menu();

	myhero->Q()->RegisterSpell(SkillAimType::activate);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	event_handler<Events::onCreateObject>::add_callback(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::add_callback(Update);

	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);

	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
	event_handler<Events::onMissileRemoved>::add_callback(MissileRemove);

	event_handler<Events::onGainBuff>::add_callback(BuffGain);
	event_handler<Events::onLoseBuff>::add_callback(BuffLose);

	event_handler<Events::onGapCloser>::add_callback(GapCloser);
	event_handler<Events::onActiveSpellCast>::add_callback(onActiveSpell);

	event_handler<Events::onDraw>::add_callback(Draw);

}

void onUnload() {
	std::cout << "Unloaded..." << std::endl;

	event_handler<Events::onCreateObject>::remove_handler(CreateObject);
	event_handler<Events::onDeleteObject>::remove_handler(DeleteObject);
	event_handler<Events::onUpdate>::remove_handler(Update);
	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onMissileRemoved>::remove_handler(MissileRemove);
	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);
	event_handler<Events::onGapCloser>::remove_handler(GapCloser);
	event_handler<Events::onActiveSpellCast>::remove_handler(onActiveSpell);
	event_handler<Events::onDraw>::remove_handler(Draw);
}