#include "pch.h"
using namespace sdk;

static std::string championName = "Ezreal";
static std::string championVersion = "0.0.4";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreEzreal");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

float qdamage[5] = { 20.0, 45.0, 70.0, 95.0, 120.0 };
// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc  checks
float min_r_health = 350.0f;
float max_r_range = 3000.0f;
static bool e_gap_closer = false;

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

	if (e_gap_closer) {
		//std::cout << sender->ChampionName() << std::endl;
		auto start_pos = sender->aiPathStart();
		auto end_pos = sender->aiPathEnd();

		auto mypos = myhero->Position();

		auto start_dist = mypos.distance(start_pos);
		auto end_dist = mypos.distance(end_pos);

		auto direction = end_pos.direction(start_pos).normalize();

		auto move_pos = mypos.add(direction * myhero->E()->CastRange());

		if (myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
			&& mypos.distance(end_pos) < 700.0f && start_dist > end_dist) {
			myhero->E()->Cast(move_pos);
		}
	}
}

/*
 * MissileCreate
 */
void MissileCreate(std::shared_ptr<MissileInstance> missile) {
}

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr<MissileInstance> missile) {
}

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
 * Combo
 */
void Combo() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr)
		return;

	if (debug) {
		std::cout << myhero->Q()->MissileSpeed()<< " : " << myhero->Q()->CastRange() << " : " << myhero->Q()->CastDelay() << " : " << myhero->Q()->LineWidth() << std::endl;
	}

    if (settings->combo.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && myhero->inRange(target.get(), myhero->W()->CastRange())) {

		auto predictionW = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->W()->MissileSpeed(), myhero->W()->CastRange(), myhero->W()->CastDelay() / 1000.0f);

		if (predictionW.isValid())
			myhero->W()->Cast(predictionW);
	}
	else if (settings->combo.useq &&  myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0);

		if (prediction.isValid()) {
			auto collide = predictionmanager->WillCollide(myhero->Position(), prediction, myhero->Q()->LineWidth()+40.0f, target.get(),true,true);

			if(!collide)
			myhero->Q()->Cast(prediction);
		}
	}

	else if (settings->combo.user  && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts()
		&& myhero->inRange(target.get(), max_r_range) && target->CurrentHealth() < min_r_health){
		myhero->R()->Cast(target);
	}
}
/*
 * Q_Logic
 */
void Q_Logic() {
	auto target = targetselector->GetLastHitTarget(myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	auto prediction = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), 2000.0f,
		myhero->Q()->CastRange(), 0.25);

	if (myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->R()->CastRange())) {
		if (prediction.isValid())
			myhero->Q()->Cast(prediction);
	}
}

void Q_Logic_LaneClear() {
	auto target = targetselector->GetLastHitTarget(myhero->Q()->CastRange());

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->R()->CastRange())) {
	
		myhero->Q()->Cast(target);
	}
}

/*
 * LastHit
 */
void LastHit() {

	float ad_dmg = qdamage[myhero->Q()->Level()-1]+ (1.30 * myhero->TotalRawAttackDamage()) + (0.15 * myhero->AbilityPower());

	auto target = targetselector->GetLastHitTarget(myhero->Q()->CastRange(), ad_dmg);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->lasthit.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast(target);

	}
}
/*
 * Herass
 */
void Herass() { 
	
	LastHit();
	
	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	if (target == nullptr  || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->herass.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast(target);
	}
}

/*
 * LaneClear
 */
void LaneClear() {
	if (settings->laneclear.useq)
		Q_Logic_LaneClear();
}


/*
 * Update
 */
void Update() {
	if (orbwalker->Mode() == OrbwalkModes::COMBO) {
		Combo();
	}
	else if (orbwalker->Mode() == OrbwalkModes::HERASS 
		&& myhero->CurrentManaPercent() >= settings->herass.min_mana_percent) {
		Herass();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LANECLEAR 
		&& myhero->CurrentManaPercent() >= settings->laneclear.min_mana_percent) {
		LaneClear();
	}
	else if (orbwalker->Mode() == OrbwalkModes::LASTHIT 
		&& myhero->CurrentManaPercent() >= settings->lasthit.min_mana_percent) {
		LastHit();
	}
}

/*
 * Draw
 */
void Draw() {
	

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
	auto root_node = menu->CreateSubmenu("CoreEzreal");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use W", &settings->combo.usew);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use E On GapCloser", &e_gap_closer);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);
	sub_combo->AddSliderFloat("Use R Healt <=", min_r_health, 100.0f, 2000.0f, {}, true);
	sub_combo->AddSliderFloat("R Max Range", max_r_range, 1000.0f, 15000.0f, {}, true);
	
	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	min_r_health = settings->LoadCustomFloat("Use R Healt <=")>0.0f ? settings->LoadCustomFloat("Use R Healt <=") : min_r_health;
	
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");
	e_gap_closer = settings->LoadCustomBool("Use E On GapCloser");

	Menu();

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.25, 0.0, 0.0, 2000.0f);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target, 0.25, 0.0, 0.0, 1700.0f);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

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