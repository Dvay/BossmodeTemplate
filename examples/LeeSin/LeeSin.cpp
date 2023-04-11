// dllmain.cpp : Definiert den Einstiegspunkt für die DLL-Anwendung.
#include "pch.h"
#include <map>
#include <set>
#include <queue>
#include <iomanip>
#include "LeeSin.h"

using namespace sdk;

static std::string championName = "LeeSin";
static std::string championVersion = "0.0.7";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreLeeSin");
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
static bool is_dashing_q2 = false;
static bool did_dash = false;
static Vector3 flash_position;
static Vector3 insecPos;
static int insec_key = 0x47;
static int attack_count = 0;
static bool use_wardjump_on_flee = 0;
static int selected_insec = 0;

MissileInstance* miss = nullptr;
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}

void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) {

	attack_count++;
}

void SpellCast(GameObjectInstance* sender) {
	//	if (sender->NetworkId() == myhero->NetworkId()) {
			//	std::cout << sender->ActiveSpellCast()->SpellName()  << "  :  ";
		//}

		/*if (sender->ActiveSpellCast()->Slot() == 0 && sender->Team() != myhero->Team() && myhero->Position().distance(sender->ActiveSpellCast()->EndPosition()) < 200 && myhero->E()->isReady()) {
			auto target = targetselector->GetTargetByIndex(sender->Index());

			if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
				myhero->E()->Cast(target);
			}
		}*/
}

void GapCloser(GameObjectInstance* sender) {
	//	if (sender->NetworkId() == myhero->NetworkId()) {
//	std::cout << sender->Name() << std::endl;
		//}
	if (orbwalker->Mode() == OrbwalkModes::NONE)
		return;
}
Vector3 evade_pos;
Vector3 evade_pos2;

void ActiveSpell(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> active_spell) {

//	std::cout << active_spell->GetSpellName() << std::endl;

	//if (active_spell->GetSpellName() != "BlindMonkQOne")
	/*if (active_spell->GetSpellName() != "MorganaQ")
		return;

	
	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	Vector3 colliding_point(0, 0, 0);
	if (predictionmanager->WillCollide(evade_pos, active_spell->GetStartPosition(), active_spell->GetEndPosition(), myhero.get(), active_spell->GetLineWidth())) {

		auto p = target->Position();
		auto direction = p.direction(evade_pos);


		auto o_norm = direction.perpendicular().normalize();

		evade_pos2 = evade_pos.add(o_norm.scale(200.0f));
	
		std::cout << "yes" << std::endl;
		//evade_pos = predictionmanager->dodge_missile(active_spell->GetMissleSpeed(), myhero->MoveSpeed(), active_spell->GetStartPosition(), active_spell->GetEndPosition(), active_spell->GetCastRange(), active_spell->GetLineWidth(), myhero->Position(), myhero->BoundingRadius());

		//evade_pos = predictionmanager->GetBestEvadePosition(colliding_point, active_spell->GetLineWidth(), myhero->Position(), myhero->BoundingRadius(), myhero->MoveSpeed(), 0.0);

		//auto direction_nomr = myhero->Position().direction(evade_pos).normalize();
		//evade_pos = evade_pos.add(direction_nomr.scale(900.0f));
		orbwalker->SetOrbwalkingPoint(&evade_pos2);
		
		//std::cout << evade_pos.x << std::endl;

	}*/


}

void MissileCreate(std::shared_ptr<MissileInstance> missile) {

	//
	//auto from = missile->Position();
	//auto p = myhero->Position();

/*		if (missile->Name() != "MorganaQ")
			return;

		std::cout << missile->Name() << std::endl;

		Vector3 colliding_point(0,0,0);
		if (predictionmanager->WillCollide(colliding_point,missile->StartPos(), missile->EndPos(), myhero.get(), missile->LineWidth())) {

			 evade_pos = predictionmanager->GetBestEvadePosition(colliding_point, missile->LineWidth(), myhero->Position(), myhero->BoundingRadius(), myhero->MoveSpeed(), 0.0);
		
			 auto direction_nomr = myhero->Position().direction(evade_pos).normalize();
			 evade_pos = evade_pos.add(direction_nomr.scale(900.0f));
			orbwalker->SetOrbwalkingPoint(&evade_pos);
			
			std::cout << evade_pos.x << std::endl;

		}*/

	/*	all_missiles.push_back(missile);
		auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, 2000.0f);

		if (target == nullptr)
			return;

		auto s = missile->StartPos();
		auto t = missile->EndPos();
		std::cout << predictionmanager->WillCollide(s, t, target.get(), missile->LineWidth()) << std::endl;;
		std::cout << myhero->Position().distance(target->Position()) << std::endl;
		std::cout << myhero->Q()->CastRange() << std::endl;*/
		//if (missile->Team() != myhero->Team() && myhero->Position().distance(missile->EndPos()) < 200 && myhero->E()->isReady()) {
	   //    auto target = targetselector->GetTargetByIndex(missile->SourceIndex());

		//	if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
			//	myhero->E()->Cast(target);
		//	}
		//}
		//if (missile->LineWidth() > 0)
		//	std::cout << "Missile: " << missile->Team() << " : " << missile->Name() << std::endl;
}

void MissileRemove(std::shared_ptr<MissileInstance> missile) {

	
//	if (missile->Name() != "MorganaQ")
	//	return;
	orbwalker->SetOrbwalkingPoint(nullptr);
	//all_missiles.clear();
	//std::cout << "Missile remove: " << missile->Name() << std::endl;

//if (missile->Team() != myhero->Team() && myhero->Position().distance(missile->EndPos()) < 200 && myhero->E()->isReady()) {
//    auto target = targetselector->GetTargetByIndex(missile->SourceIndex());

 //	if (target != nullptr && myhero->inRange(target, myhero->E()->CastRange())) {
	 //	myhero->E()->Cast(target);
 //	}
 //}
 //if (missile->LineWidth() > 0)
 //	std::cout << "Missile: " << missile->Team() << " : " << missile->Name() << std::endl;
}

void BuffGain(std::shared_ptr<BuffInstance> buff) {
//	if (buff->SourceIndex() != myhero->Index())
//		return;

	//std::cout << "Buff Gain: " << buff->Name() << std::endl;
}

void BuffLose(std::shared_ptr<BuffInstance> buff) {
	if (buff->SourceIndex() != myhero->Index())
		return;

	//std::cout << "Buff Lose: " << buff->Name()  << std::endl;
}

void CreateObject(std::shared_ptr<GameObjectInstance> obj) {
	//if (obj->ObjectName() == "DravenSpinningReturn") {
		//	std::cout << "Created: " << obj->ObjectName() << " : " << (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now() - time1).count()) << std::endl;
//	}
}

void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {
	//if (obj->ObjectName().find("Draven") != std::string::npos)
	//std::cout << "Deleted: " << obj->ObjectName() << std::endl;
}

Vector3 GetDashableCreep(Vector3  from, Vector3 to, float angle) {
	for (auto& object : targetselector->m_all_units) {
		auto obj = object.second->Position();

		if (!object.second->isValid() || !object.second->isAlly() || from.distance(obj) > myhero->W()->CastRange())
			continue;

		if (predictionmanager->isBetween(from, to, obj, angle)) {
			return obj;
		}
	}

	return Vector3();
}


Vector3 positionLineDraw;
bool ulti_casted = false;

void InSec_Logic()
{

	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	auto q_buff = target->hasBuff(buff, "blindmonkqone");
	auto q_buff2 = myhero->hasBuff(buff, "blindmonkqmanager"); // to make sure Q2 is not casted when Q is on the wrong target


	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f);
	auto mypos = myhero->Position();

	if (settings->combo.useq && myhero->Q()->isReady() && !q_buff && !q_buff2 && pos.isValid() && myhero->Position().distance(pos) <= myhero->Q()->CastRange() && !predictionmanager->WillCollide(mypos, pos, myhero->Q()->LineWidth()+20.0f,target.get(),true,true)) {
		myhero->Q()->Cast(pos);
	}

	if (settings->combo.useq && myhero->Q()->isReady() && q_buff && myhero->inRange(target.get(), myhero->Q()->CastRange() + 50.0f)) {
		auto direction = insecPos.sub(target->Position()).normalize();
		flash_position = target->Position().add(direction.scale(200));

		myhero->Q()->Cast(target);
		is_dashing_q2 = true;
	}

	if (is_dashing_q2 && !q_buff) {
		is_dashing_q2 = false;
		did_dash = true;
	}

	if ((did_dash && myhero->R()->isReady() && myhero->SUM2()->isReady()) || (ulti_casted && myhero->R()->isReady())) {

		if (!ulti_casted)
			myhero->SUM2()->Cast(flash_position);

		myhero->R()->Cast(target);
		ulti_casted = true;

	}
	else if (!myhero->R()->isReady())
		ulti_casted = false;

	did_dash = false;

}


Vector3 GetClosesAlly() {


	auto mypos = myhero->Position();
	for (auto& unit : targetselector->m_all_heros) {

		if (!unit->isValid() || !unit->isAlly()) continue;

		auto t_pos = unit->Position();
		if (mypos.distance(t_pos) < 900.0f)
			return t_pos;

	}

	return Vector3();
}

Vector3 GetClosesTower() {

	auto mypos = myhero->Position();
	for (auto& tower : targetselector->m_all_turrets) {

		if (!tower.second->isValid() || !tower.second->isAlly()) continue;

		auto t_pos = tower.second->Position();
		if (mypos.distance(t_pos) < 1700.0f)
			return t_pos;

	}

	return Vector3();
}

void Shot_Distance_insec()
{

	if (!myhero->R()->isReady())
		return;

	auto yellow_trinket_slot = myhero->GetItemSlot(3340);

	if (yellow_trinket_slot == 0)
		return;

	auto yellow_trinke_item = myhero->GetItemBySlot(yellow_trinket_slot);

	auto target = targetselector->GetTarget(TargetSelectorTypes::distance_to_champ, 600.0f);

	if (target == nullptr)
		return;

	Vector3 insec_position;
	if (selected_insec == 2)
		insec_position = insecPos;
	else if (selected_insec == 0) {
		insec_position = GetClosesTower();
		insecPos = insec_position;

	}
	else if (selected_insec == 1)
		insec_position = GetClosesAlly();

	if(!insec_position.isValid())
		return;

	auto direction = insec_position.sub(target->Position()).normalize();
	flash_position = target->Position().add(direction.scale(200));

	auto q_buff = target->hasBuff(buff, "blindmonkqone");
	auto q_buff2 = myhero->hasBuff(buff, "blindmonkqmanager"); // to make sure Q2 is not casted when Q is on the wrong target


	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f);
	auto mypos = myhero->Position();
	static bool w_casted_to_pos = false;
	static bool second_q_casted = false;
	static bool first_e_casted = false;


	if(!myhero->R()->isReady() && (game->GameTime() - myhero->R()->GetLastTimeCasted() > 500.0f) && myhero->Q()->isReady() && q_buff)
	{
		myhero->Q()->Cast(target);
		second_q_casted = true;
	}

	if(second_q_casted && myhero->E()->isReady() && myhero->inRange(target.get(),350.0f))
	{
		myhero->E()->Cast();
		second_q_casted = false;
		first_e_casted = true;
	}


	if (first_e_casted && myhero->E()->isReady() && myhero->inRange(target.get(), 350.0f))
	{
		myhero->E()->Cast();
		first_e_casted = false;
	}

	if (settings->combo.useq && myhero->Q()->isReady() && myhero->W()->isReady() && yellow_trinke_item->isReady() && !q_buff && !q_buff2 && pos.isValid() && myhero->Position().distance(flash_position) <= 625.0 && !predictionmanager->WillCollide(mypos, pos, myhero->Q()->LineWidth(),target.get(),true,true)) {
		myhero->Q()->Cast(pos);
		Sleep(100);
		yellow_trinke_item->Cast(flash_position);
		myhero->W()->Cast(flash_position);
	

	}

	if (!w_casted_to_pos && myhero->aiIsDashing() && myhero->R()->isReady())
		w_casted_to_pos = true;

	if(w_casted_to_pos && !myhero->aiIsDashing() && myhero->R()->isReady())
	{
		myhero->R()->Cast(target);
		w_casted_to_pos = false;

	}
}
void Combo() {


	auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	 
	auto passive = myhero->hasBuff(buff, "blindmonkpassive_cosmetic");
	auto q_buff = target->hasBuff(buff, "blindmonkqone");
	auto q_buff2 = myhero->hasBuff(buff, "blindmonkqmanager");
	auto w_shield = myhero->hasBuff(buff, "blindmonkwoneshield");
	auto e_one = target->hasBuff(buff, "blindmonkeone");
	static int spellOrder = 0;

	//blindmonkwoneshield
	//blindmonkeone

	if (settings->combo.useq && !passive && myhero->Q()->isReady() && spellOrder == 0 && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()  && myhero->inRange(target.get(), myhero->AttackRange()+200.0f)) {
		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f);

	

		if (pos.isValid()) {
			myhero->Q()->Cast(pos);
			if (q_buff)
				spellOrder = 1;
		}


	}
	else if (!e_one && settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()  && myhero->inRange(target.get(), myhero->Q()->CastRange()) && !myhero->inRange(target.get(), myhero->AttackRange()+200.0f)) {
		auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f);

		if (pos.isValid()) {
			myhero->Q()->Cast(pos);


			spellOrder = 1;
		}

	}
	else if (settings->combo.usew && !passive && myhero->W()->isReady() && spellOrder == 1 && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && myhero->inRange(target.get(), myhero->W()->CastRange())) {

		if (w_shield)
			spellOrder = 2;

		myhero->W()->Cast(myhero);


	}

	else if (settings->combo.usee && !passive && myhero->E()->isReady() && spellOrder == 2 && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && myhero->inRange(target.get(), myhero->E()->CastRange())) {

		if (e_one)
			spellOrder = 0;

		myhero->E()->Cast();

	
	}

	auto game_time = game->GameTime();
	// If the current spell cannot be casted, move on to the next spell in the priority list
	if (spellOrder == 0 && !myhero->Q()->isReady() && (game_time - myhero->Q()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 1;
		attack_count = 0;
	}
	else if (spellOrder <= 1 && !myhero->W()->isReady() && (game_time - myhero->W()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 2;
		attack_count = 0;
	}
	else if (spellOrder <= 2 && !myhero->E()->isReady() && (game_time - myhero->E()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 0;
		attack_count = 0;
	}

	// Reset spell order to 0 if all spells have been casted
	//if (spellOrder == 3) {
//		spellOrder = 0;
	//}




//	InSec_Logic();
}

void Flee() {
	if (settings->combo.usew && myhero->W()->isReady()) {
		auto pos = GetDashableCreep(myhero->Position(), game->GetMousePosition(), 20.0f);

		if (pos.isValid())
			myhero->W()->Cast(pos);
		else if(use_wardjump_on_flee) {

			auto yellow_trinket_slot = myhero->GetItemSlot(3340);

			if (yellow_trinket_slot == 0)
				return;

			auto yellow_trinke_item = myhero->GetItemBySlot(yellow_trinket_slot);

			if (yellow_trinke_item->isReady()) {

				auto mouse_pos = game->GetMousePosition();
				auto direction = mouse_pos.direction(myhero->Position()).normalize();
				auto ward_pos = myhero->Position().add(direction.scale(600));

				yellow_trinke_item->Cast(ward_pos);
			}

		}
	}
}
void Herass() { }

void LaneClear() { 

	auto target = targetselector->GetLaneClearTarget(myhero->Q()->CastRange());

	if (target == nullptr)
		return;

	target->GetBuffManager()->ReadBuffs();


	auto passive = myhero->hasBuff(buff, "blindmonkpassive_cosmetic");
	auto q_buff = target->hasBuff(buff, "blindmonkqone");
	auto q_buff2 = myhero->hasBuff(buff, "blindmonkqmanager");
	auto w_shield = myhero->hasBuff(buff, "blindmonkwoneshield");
	auto e_one = target->hasBuff(buff, "blindmonkeone");
	static int spellOrder = 0;

	//blindmonkwoneshield
	//blindmonkeone

	if (settings->laneclear.useq && !passive && myhero->Q()->isReady() && spellOrder == 0 && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->inRange(target.get(), myhero->AttackRange() + 200.0f)) {
		//auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f, myhero->Q()->LineWidth());

			myhero->Q()->Cast(target);
			if (q_buff)
				spellOrder = 1;
		


	}
	else if (!e_one && settings->combo.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && myhero->inRange(target.get(), myhero->Q()->CastRange()) && !myhero->inRange(target.get(), myhero->AttackRange() + 200.0f)) {

		myhero->Q()->Cast(target);


		spellOrder = 1;

	}
	else if (settings->laneclear.usew && !passive && myhero->W()->isReady() && spellOrder == 1 && myhero->CurrentMana() >= myhero->W()->GetManaCosts() && myhero->inRange(target.get(), myhero->W()->CastRange())) {

		if (w_shield)
			spellOrder = 2;

		myhero->W()->Cast(myhero);


	}

	else if (settings->laneclear.usee && !passive && myhero->E()->isReady() && spellOrder == 2 && myhero->CurrentMana() >= myhero->E()->GetManaCosts() && myhero->inRange(target.get(), myhero->E()->CastRange())) {

		if (e_one)
			spellOrder = 0;

		myhero->E()->Cast();


	}

	auto game_time = game->GameTime();
	// If the current spell cannot be casted, move on to the next spell in the priority list
	if (spellOrder == 0 && !myhero->Q()->isReady() && (game_time - myhero->Q()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 1;
		attack_count = 0;
	}
	else if (spellOrder <= 1 && !myhero->W()->isReady() && (game_time - myhero->W()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 2;
		attack_count = 0;
	}
	else if (spellOrder <= 2 && !myhero->E()->isReady() && (game_time - myhero->E()->GetLastTimeCasted() > 1000.0f) && attack_count >= 2) {
		spellOrder = 0;
		attack_count = 0;
	}


	if (spellOrder == 1 && !settings->laneclear.usew)
		spellOrder = 2;

	if (spellOrder == 2 && !settings->laneclear.usee)
		spellOrder = 0;

	if (spellOrder == 0 && !settings->laneclear.useq)
		spellOrder = 1;

}

void LastHit() {

	auto target = targetselector->GetLastHitTarget(1000.0f);

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(), myhero->AttackRange())))
		return;

	if (settings->lasthit.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
	
			myhero->Q()->Cast(target);
	
	}
	else if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
			myhero->E()->Cast();
	}


 }

void Update() {
	//	bool hasBuff = myhero->hasBuff(buff, "DravenSpinning", true);

	if(orbwalker->Mode() == OrbwalkModes::CUSTOM1)
	{
		Shot_Distance_insec();
	}

	else if (orbwalker->Mode() == OrbwalkModes::COMBO) {
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
	else if (orbwalker->Mode() == OrbwalkModes::FLEE) {
		Flee();
	}
}

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

		//auto positionm = myhero->Position();
		//auto mousepos = game->GetMousePosition();

		//drawmanager->DrawCircle3D(positionm, myhero->AttackRange(), false, 40, RGBA_COLOR(255, 222, 120, 255), 2);

		//auto tppos = target->Position();
	/*	for (auto& object : targetselector->m_all_units) {
			if (!object.second->isValid() || object.second->isAlly())
				continue;

			auto obj = object.second->Position();
			if (predictionmanager->isBetween(positionm, mousepos, obj, 20.0f)) {
				drawmanager->DrawCircle3D(object.second->Position(), 10, true, 20, RGBA_COLOR(255, 222, 120, 255), 2);
			}

			//	if (predictionmanager->WillCollide(positionm, tppos, object.second.get(), myhero->Q()->LineWidth())) {
					//	drawmanager->DrawCircle3D(object.second->Position(), 30, true, 20, RGBA_COLOR(0, 222, 120, 255), 2);
		}*/

		static int click_count = 0;
		static float gametime = 0;
		if (!menu->isOpen()) {
			if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) {
				click_count++;
				gametime = game->GameTime();
				if (click_count >= 2) {
					insecPos = game->GetMousePosition();
					click_count = 0;
				}

				while (GetAsyncKeyState(VK_LBUTTON));
			}

			if (gametime > 0 && game->GameTime() - gametime > 300.0f) {
				click_count = 0;
				gametime = 0;
			}

		}

		if (insecPos.isValid())
			drawmanager->DrawCircle3D(insecPos, 20, false, 15, RGBA_COLOR(0, 222, 120, 255), 2);

		auto target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange());

		if (target == nullptr)
			return;

		auto direction = insecPos.sub(target->Position()).normalize();
		flash_position = target->Position().add(direction.scale(200));

		if (flash_position.isValid())
			drawmanager->DrawCircle3D(flash_position, 20, true, 20, RGBA_COLOR(255, 222, 120, 255), 2);

	//	auto p = predictionmanager->GetLineAOECastPosition(target.get(), myhero->Q()->CastDelay() / 1000.0f, myhero->Q()->LineWidth(), 2000.0f, myhero->Position(), myhero->Q()->CastRange(), 1);

		//positionLineDraw = p.castPosition;
		//positionLineDraw = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay() / 1000.0f, myhero->Q()->LineWidth());
		      


		//if (positionLineDraw.isValid())
		//	drawmanager->DrawCircle3D(positionLineDraw, 18, true, 5, RGBA_COLOR(255, 0, 0, 255), 2);

		//	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->CastRange(), myhero->Q()->MissileSpeed(), 0.25f, myhero->Q()->LineWidth());
		//	auto pos = predictionmanager->GetPredictedPositionSkillShot(myhero.get(), target.get(), myhero->Q()->MissileSpeed(), myhero->Q()->CastRange(), myhero->Q()->CastDelay(), myhero->Q()->LineWidth());
		//	drawmanager->DrawCircle3D(pos, 50, true, 20, RGBA_COLOR(0, 222, 120, 255), 2);

			//auto current_path = target->aiFullPath();

		//	auto norm_direction = target->Position().add(target->MoveDirection().normalize().scale(200));

			//std::cout << pos.x << " : " << pos.y << " : " << pos.z << std::endl;
		//	if(current_path.size() > 1)
			//drawmanager->DrawCircle3D(current_path[1], 30, true, 20, RGBA_COLOR(0, 222,120, 255), 2);
			//if (current_path.size() > 2)
		//	if(pos.isValid())
			//if(myhero->Position().distance(pos) <= myhero->Q()->CastRange())
	
}

void Menu() {
	auto root_node = menu->CreateSubmenu("CoreLeeSin");

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

	auto sub_insec = menu->CreateSubmenu("Insec");
	sub_insec->AddDropDown("Insec Position", selected_insec, { "To Tower", "To Ally", "To Click Position" },{},true);
	sub_insec->AddHotkey("Insec", insec_key);

	sub_combo->addItem(sub_insec);

	orbwalker->RegisterCustomKey("Insec",insec_key);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use W", &settings->laneclear.usew);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);


	auto sub_misc = menu->CreateSubmenu("Misc");
	sub_lasthit->AddCheckbox("Ward Jump while flee", &use_wardjump_on_flee,{}, true);

	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });


	menu->addItem(root_node);
}

void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target, 0.25);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::activate);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

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
	event_handler<Events::onActiveSpellCast>::add_callback(ActiveSpell);

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
	event_handler<Events::onActiveSpellCast>::remove_handler(ActiveSpell);

	event_handler<Events::onDraw>::remove_handler(Draw);
}