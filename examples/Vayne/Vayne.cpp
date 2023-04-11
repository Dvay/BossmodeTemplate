#include "pch.h"
#include <map>
#include <set>
#include <queue>
#include <iomanip>

using namespace sdk;

static std::string championName = "Vayne";
static std::string championVersion = "0.0.5";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreVayne");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;

// Misc checks
static bool did_attack_after_last_q = true;
static bool draw_condemn_end_position = true;
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
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) {

	did_attack_after_last_q = true;
}

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

	if (e_gap_closer && settings->combo.usee && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && sender->Team() != myhero->Team() && myhero->E()->isReady()) {
		auto target = targetselector->GetTargetByIndex(sender->Index());

		if (target != nullptr && myhero->inRange(target.get(), myhero->E()->CastRange())) {
			myhero->E()->Cast(target);
		}
	}

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


Vector3 GetWallSpots( Vector3 positionA, float radius, int numberOfSamples) {


	Vector3 forward(1, 0, 0); // Initial direction

	float angleStep = 360.0f / numberOfSamples;
	for (int i = 0; i < numberOfSamples; ++i) {
		float yawAngle = i * angleStep;
		Vector3 direction = forward.rotate_y(yawAngle);
		Vector3 positionToCheck = positionA + direction * radius;


		if (positionToCheck.isValid() && predictionmanager->isWall(positionToCheck)) {

			auto direction_to_champ = positionA.direction(positionToCheck).normalize();
			auto tumlepos = positionA + direction_to_champ.scale(myhero->E()->CastRange());

			if (myhero->Position().distance(tumlepos) <= 270.0f) {

				return tumlepos;
				//myhero->Q()->Cast(tumlepos);

			}
		//	drawmanager->DrawCircle3D(tumlepos, 5, true, 5, RGBA_COLOR(255, 0, 0), 1);

		}
	}

	
	return Vector3();
}
/*
* E_Logic
*/
bool E_Logic(std::shared_ptr<GameObjectInstance> & target) {

	//auto position = target->aiServerPosition();

	auto position = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(),
		myhero->E()->MissileSpeed(), myhero->E()->CastRange(),
		myhero->E()->CastDelay() / 1000.0f
	);

	if(!position.isValid())
		return false;

	auto myhero_pos = myhero->Position();
	auto norm_direction_hero_target = target->Position().direction(myhero_pos).normalize();

	auto E_Pos1 = position + norm_direction_hero_target.scale(200.0f + target->BoundingRadius() / 2.0f);
	auto E_Pos2 = position + norm_direction_hero_target.scale(475.0f + target->BoundingRadius() / 2.0f);

	if (predictionmanager->isWall(E_Pos1) || predictionmanager->isWall(E_Pos2)) {
		return true;
	}
	return false;
}

/*
* Combo
*/
void Combo() {

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr)
		return;


	//auto E_pos = GetWallSpots(target->Position(), 475.0f, 360.0f);

	if (did_attack_after_last_q && settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& !myhero->CanAttack() && !myhero->isAttacking() && myhero->inRange(target.get(), myhero->AttackRange())) {

		myhero->Q()->Cast();
		did_attack_after_last_q = false;
	}
	else if (settings->combo.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {

		if(E_Logic(target))
			myhero->E()->Cast(target);
	}
	else if (settings->combo.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->AttackRange())) {

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

	if (settings->lasthit.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
			myhero->Q()->Cast();
	}
}

/*
* Herass
*/
void Herass() { 

	LastHit();

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr  || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->herass.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		myhero->Q()->Cast();
	}
}

/*
* LaneClear
*/
void LaneClear() {

	auto target = targetselector->GetLaneClearTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(),myhero->AttackRange())))
		return;

	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
			myhero->Q()->Cast();
	}
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

		
		
		auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->E()->CastRange());

		if (target == nullptr)
			return;



		if (draw_condemn_end_position) {
			auto position = target->aiServerPosition();

			auto mp = myhero->Position();
			auto norm_direction_hero_target = target->Position().direction(mp).normalize();

			auto E_Pos = position + norm_direction_hero_target.scale(475.0f);

			drawmanager->DrawCircle3D(E_Pos, 30, true, 20, RGBA_COLOR(0, 222,120, 255), 2);
		}


		if (draw_q_range && myhero->Q()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->Q()->CastRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 2);
		}

		if (draw_w_range && myhero->W()->Level() > 0) {
			drawmanager->DrawCircle3D(myhero->Position(), myhero->AttackRange(), false, 60, RGBA_COLOR(140, 142, 100, 255), 2);
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
	auto root_node = menu->CreateSubmenu("CoreVayne");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw condemn end Position", &draw_condemn_end_position);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);

	auto sub_combo = menu->CreateSubmenu("Combo");
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use E On GapCloser", &e_gap_closer);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);

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


	myhero->Q()->RegisterSpell(SkillAimType::activate);
	//myhero->W()->RegisterSpell(SkillAimType::activate);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target,0.25,0.0,0.0,2200.0);
	myhero->R()->RegisterSpell(SkillAimType::activate);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	e_gap_closer = settings->LoadCustomBool("Use E On GapCloser");

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