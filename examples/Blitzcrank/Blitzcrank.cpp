#include "pch.h"
using namespace sdk;

static std::string championName = "Blitzcrank";
static std::string championVersion = "0.0.4";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreBlitzcrank");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc  checks
bool did_grap = false;
int r_min_enemies = 1;


/*
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}

/*
 * AfterAttack
 */
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) { }

/*
 * SpellCast
 */
void SpellCast(GameObjectInstance* sender) { }

/*
 * GapCloser
 */
void GapCloser(GameObjectInstance* sender) {
	if (orbwalker->Mode() == OrbwalkModes::NONE)
		return;
}

/*
 * MissileCreate
 */
void MissileCreate(std::shared_ptr<MissileInstance> missile) { }

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr<MissileInstance> missile) { }

/*
 * BuffGain
 */
void BuffGain(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() == myhero->Index())
		return;
}

/*
 * BuffLose
 */
void BuffLose(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
		return;
}

/*
 * CreateObject
 */
void CreateObject(std::shared_ptr<GameObjectInstance> obj) { }

/*
 * DeleteObject
 */
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) { }



/*
 * EnemiesIn_R_Range
 */
int EnemiesIn_R_Range() {
	int count = 0;
	for (auto& obj : targetselector->m_all_heros) {
	
		if (!obj->isValid() || obj->isAlly())
			continue;

		if (myhero->inRange(obj.get(), myhero->R()->CastRange())) {
			count++;
		}
	}
	return count;
}

/*
 * Combo
 */
void Combo() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange() - 110.0f);

	if (target == nullptr)
		return;

	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange()-target->BoundingRadius()-50.0f, myhero->Q()->CastDelay() / 1000.0f); 
	auto mypos = myhero->Position();

	auto has_powerfist = target->hasBuff(buff, "powerfist");
	
	if (has_powerfist)
		return;

	auto has_rocktGrab = target->hasBuff(buff, "rocketgrab2");
			

	//Q
	if (settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && pos.isValid()
		&& myhero->Position().distance(pos) <= myhero->Q()->CastRange() 
		&& !predictionmanager->WillCollide(mypos, pos, myhero->Q()->LineWidth()+25.0f, target.get(),true,true)) {
		myhero->Q()->Cast(pos);
	}
	// W
	else if (settings->combo.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->AttackRange())) {
		myhero->W()->Cast();
	}
	// E
	else if (settings->combo.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& (has_rocktGrab || myhero->inRange(target.get(), myhero->AttackRange()))) {
		did_grap = true;
		myhero->E()->Cast();
	}
	// R
	else if (settings->combo.user && myhero->R()->isReady() && did_grap && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& EnemiesIn_R_Range() >= r_min_enemies && myhero->inRange(target.get(), myhero->R()->CastRange())) {
		did_grap = false;
		myhero->R()->Cast();
	}
}

/*
 * Herass 
 */
void Herass() { }

/*
 * LaneClear
 */
void LaneClear() {

	auto target = targetselector->GetLaneClearTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;
		
	if (settings->laneclear.usee && myhero->E()->isReady() && myhero->inRange(target.get(), myhero->E()->CastRange())) {
		myhero->E()->Cast(target);
	
	}
	else if (settings->laneclear.user && myhero->R()->isReady() && myhero->inRange(target.get(), myhero->R()->CastRange())) {
		myhero->R()->Cast();
	}
}

/*
 * LastHit
 */
void LastHit() {

	auto target = targetselector->GetLastHitTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->laneclear.usee && myhero->E()->isReady() && myhero->inRange(target.get(), myhero->E()->CastRange())) {
			myhero->E()->Cast(target);
	}
}

/*
 * Update
 */
void Update() {
	if (orbwalker->Mode() == OrbwalkModes::COMBO) {
		Combo();
	}
	else if (orbwalker->Mode() == OrbwalkModes::HERASS && myhero->CurrentManaPercent() >= settings->herass.min_mana_percent) {
		Herass();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LANECLEAR && myhero->CurrentManaPercent() >= settings->laneclear.min_mana_percent) {
		LaneClear();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LASTHIT && myhero->CurrentManaPercent() >= settings->lasthit.min_mana_percent) {
		LastHit();
	}
}

/* 
 * Draw
 */
void Draw() {
	

		// Debug blitz Q Prediction
		if (debug) {
			auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

			if (target == nullptr)
				return;

			auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(),
				myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(),
				myhero->Q()->CastDelay() / 1000.0f			);
			if (pos.isValid())
				drawmanager->DrawCircle3D(pos, 30, true, 20, RGBA_COLOR(0, 222, 120, 255), 2);
		}

		if (!myhero->isAlive())
			return;

		if (draw_q_range && myhero->Q()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->Q()->CastRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 2);
		}

		if (draw_w_range && myhero->W()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->W()->CastRange(), false, 60, RGBA_COLOR(140, 142, 100, 255), 2);
		}

		if (draw_e_range && myhero->E()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->E()->CastRange(), false, 60, RGBA_COLOR(78, 142, 100, 255), 2);
		}

		if (draw_r_range && myhero->R()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->R()->CastRange(), false, 60, RGBA_COLOR(150, 100, 100, 255), 2);
		}
	
}

/*
 * Menu
 */
void Menu() {

	auto root_node = menu->CreateSubmenu("CoreBlitzcrank");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderInt("Use R if Enemies >=", r_min_enemies, 1, 5);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);
	sub_herass->AddCheckbox("Use R", &settings->herass.user);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);
	sub_laneclear->AddCheckbox("Use R", &settings->laneclear.user);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);

	root_node->addItems({ sub_combo,sub_herass,sub_laneclear,sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.25, 1000.0f);
	myhero->W()->RegisterSpell(SkillAimType::activate);
	myhero->E()->RegisterSpell(SkillAimType::activate);
	myhero->R()->RegisterSpell(SkillAimType::activate, 0.25);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	r_min_enemies = (settings->LoadCustomInt("Use R if Enemies >=") > 0 ? settings->LoadCustomInt("Use R if Enemies >=") : r_min_enemies);

	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	Menu();

	event_handler<Events::onCreateObject>::add_callback(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::add_callback(Update);

	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);
	event_handler<Events::onSpellCast>::add_callback(SpellCast);

	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
	event_handler<Events::onMissileRemoved>::add_callback(MissileRemove);

	event_handler<Events::onGainBuff>::add_callback(BuffGain);
	event_handler<Events::onLoseBuff>::add_callback(BuffLose);

	event_handler<Events::onGapCloser>::add_callback(GapCloser);

	event_handler<Events::onDraw>::add_callback(Draw);
}

/*
 * onUnload
 */
void onUnload() {

	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " unLoaded" << std::endl;

	event_handler<Events::onCreateObject>::remove_handler(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::remove_handler(Update);

	event_handler<Events::onBeforeAttack>::remove_handler(BeforeAttack);
	event_handler<Events::onAfterAttack>::remove_handler(AfterAttack);
	event_handler<Events::onSpellCast>::remove_handler(SpellCast);

	event_handler<Events::onMissileCreate>::remove_handler(MissileCreate);
	event_handler<Events::onMissileRemoved>::remove_handler(MissileRemove);

	event_handler<Events::onGainBuff>::remove_handler(BuffGain);
	event_handler<Events::onLoseBuff>::remove_handler(BuffLose);

	event_handler<Events::onGapCloser>::remove_handler(GapCloser);

	event_handler<Events::onDraw>::remove_handler(Draw);
}