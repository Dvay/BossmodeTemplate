#include "pch.h"
#include <algorithm>
#include <vector>
using namespace sdk;

static std::string championName = "Fiora";
static std::string championVersion = "0.0.4";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreFiora");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Draw Toggle
static bool draw_q_range = false;
static bool draw_w_range = false;
static bool draw_e_range = false;
static bool draw_r_range = false;
static bool walk_vitals = false;

enum class VitalType {
	warning,
	normal,
	timeout,
	ultimate
};
enum class VitalDirection {
	NE,
	SE,
	NW,
	SW
};
struct vital {
	VitalType type;
	VitalDirection direction;
	int networkId;
	float timeout;
	float valid_time;
	std::shared_ptr < GameObjectInstance> obj = nullptr;
	std::shared_ptr < GameObjectInstance> owner = nullptr;
	float distance;

	Vector3 GetVitalPosition() {
		if (obj == nullptr)
			return Vector3();

		Vector3 position = obj->Position();

		if (direction == VitalDirection::NE) {
			position.z += 200.0f;
			return position;
		}
		else if (direction == VitalDirection::SE) {
			position.x -= 200.0f;
			return position;
		}
		else if (direction == VitalDirection::NW) {
			position.x += 200.0f;
			return position;
		}
		else if (direction == VitalDirection::SW) {
			position.z -= 200.0f;
			return position;
		}
	}
};


struct SpellBlock {

	int _networkID = 0;
	std::string _name;

	bool enable = false;
	bool on_q = false;
	bool on_w = false;
	bool on_e = false;
	bool on_r = false;

	SpellBlock(int networkID, std::string name) : _networkID(networkID), _name(name) {};
};

bool draw_vital_position = false;
bool use_q_on_flee = false;
static std::vector<SpellBlock> menu_spell_block;
static std::shared_ptr<BuffInstance> buff;
static std::vector<vital> timingout_vitals;
static std::vector<vital> arising_vitals;
static std::vector<vital> normal_vitals;
static std::vector<vital> ultimate_vitals;

bool compareByDistance(const vital& a, const vital& b) {
	return a.distance < b.distance;
}

bool AddVital(vital& _vital, std::vector<vital>& list) {
	auto vit = std::find_if(list.begin(), list.end(), [&](vital& elem) { return elem.networkId == _vital.networkId; });
	if (vit != list.end()) {
		return false;
	}

	list.push_back(_vital);
	std::sort(list.begin(), list.end(), compareByDistance);

	return true;
}

bool GetVitalDirection(std::string& name, vital& _vital) {
	bool found = false;
	if (name.find("_NE") != std::string::npos) {
		_vital.direction = VitalDirection::NE;
		found = true;
	}
	else if (name.find("_SE") != std::string::npos) {
		_vital.direction = VitalDirection::SE;
		found = true;
	}
	else if (name.find("_NW") != std::string::npos) {
		_vital.direction = VitalDirection::NW;
		found = true;
	}
	else if (name.find("_SW") != std::string::npos) {
		_vital.direction = VitalDirection::SW;
		found = true;
	}

	if (found) {
		if (name.find("_Warning") != std::string::npos) {
			_vital.type = VitalType::warning;
			return true;
		}
		else if (name.find("_FioraOnly") != std::string::npos) {
			_vital.type = VitalType::ultimate;
			return true;
		}
		else if (name.find("_Timeout") != std::string::npos) {
			_vital.type = VitalType::timeout;
			return true;
		}
		else {
			_vital.type = VitalType::normal;
			return true;
		}
	}

	return false;
}

/*
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;
}

/*
 * BeforeAttack
 */
void AfterAttack(std::shared_ptr<GameObjectInstance> target, bool* do_aa_reset) {

	auto buff1 = myhero->hasBuff(buff, "fiorae");
	auto buff2 = myhero->hasBuff(buff, "fiorae2");
//	std::cout << buff1 <<" : "<< buff2 << std::endl;
	if (orbwalker->Mode() == OrbwalkModes::COMBO && settings->combo.usee && !buff1 && !buff2 && myhero->E()->isReady() && myhero->inRange(target.get(), myhero->AttackRange())) {
		myhero->E()->Cast();
		*do_aa_reset = true;
	}
}

/*
 * ActiveSpell
 */
void ActiveSpell(std::shared_ptr<GameObjectInstance> sender, std::shared_ptr<ActiveSpellCastInstance> active_spell) {

	auto slot = active_spell->GetSlot();
	if (sender->isAlly() || (slot < 0 || slot > 3))
		return;

	for (auto& champ : menu_spell_block) {

		if (sender->NetworkId() != champ._networkID)
			continue;

		switch (slot)
		{
		case 0:
			if (champ.on_q)
				break;
			else
				return;
		case 1:
			if (champ.on_w)
				break;
			else
				return;
		case 2:
			if (champ.on_e)
				break;
			else
				return;
		case 3:
			if (champ.on_r)
				break; 
			else
				return;

		default:
			return;
		}

	}

	Vector3 colliding_point;
	auto a = predictionmanager->isObjectOnMissilePath(colliding_point, active_spell->GetStartPosition(), active_spell->GetEndPosition(), myhero.get(), active_spell->GetLineWidth() * 2.0f);

	if (a  && (active_spell->GetSlot() == 0 || active_spell->GetSlot() == 1) && myhero->W()->isReady())
		myhero->W()->Cast(sender);

	//	std::cout << active_spell->GetSpellName()  << ":" << active_spell->GetSlot()  << std::endl;
}

/*
 * SpellCast
 */
void SpellCast(GameObjectInstance* sender) {

//	std::cout << sender->Nam() << ":" << active_spell->GetSlot() << std::endl;

}

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
void MissileCreate(std::shared_ptr<MissileInstance> missile) {

	auto slot = missile->Slot();
	if (missile->Team() == myhero->Team() || (slot < 0 || slot > 3))
		return;

	auto sender = targetselector->GetTargetByIndex(missile->SourceIndex());
	if (sender == nullptr)
		return;

	for (auto& champ : menu_spell_block) {

		auto t = targetselector->GetTargetByIndex(missile->SourceIndex());
		if (t == nullptr)
			continue;

		if (t->NetworkId() != champ._networkID)
			continue;

		switch (slot)
		{
		case 0:
			if (champ.on_q)
				break;
			else
				return;
		case 1:
			if (champ.on_w)
				break;
			else
				return;
		case 2:
			if (champ.on_e)
				break;
			else
				return;
		case 3:
			if (champ.on_r)
				break;
			else
				return;

		default:
			return;
		}

	}

	Vector3 colliding_point;
	auto a = predictionmanager->isObjectOnMissilePath(colliding_point, missile->Position(), missile->EndPos(), myhero.get(), missile->LineWidth() * 2.0f);
	//std::cout << "Created Missile: " << a  << ":" << sender->Position().distance(missile->EndPos()) << " : Slot: " << missile->Slot() << ":" << missile->LineWidth() << std::endl;

	if (a && myhero->W()->isReady()) {
	
		myhero->W()->Cast(sender);
	}
}

/*
 * MissileRemove
 */
void MissileRemove(std::shared_ptr<MissileInstance> missile) {
	//std::cout << "Removed Missile: " << missile->Name() << std::endl;
}

/*
 * BuffGain
 */
void BuffGain(std::shared_ptr<BuffInstance> buff) {
	//	std::cout << "Created Buff: " << buff->Name() << std::endl;
}

/*
 * BuffLose
 */
void BuffLose(std::shared_ptr<BuffInstance> buff) {
	//	std::cout << "Removed Buff: " << buff->Name() << std::endl;
}

/*
 * CreateObject
 */
std::vector<Vector3> obj_pos;
void CreateObject(std::shared_ptr<GameObjectInstance> obj) {
	auto name = obj->ObjectName();


	// global_ss_flash.troy
	//if (name.find("KogMaw_Base_R_cas_red") != std::string::npos) {
		//std::cout << name << std::endl;
	//	obj_pos.push_back(obj->Position());
	//}

	vital vit;

	if (GetVitalDirection(name, vit)) {
		auto vit_pos = obj->Position();

		auto time = game->GameTime();
		vit.timeout = time + 1250.0f;
		vit.networkId = obj->NetworkId();
		vit.distance = myhero->Position().distance(vit_pos);
		vit.obj = obj;
		
		float temp_dist = FLT_MAX;
		for (auto& champ : targetselector->m_all_heros) {

			if (champ->isAlly() || !champ->isValid())
				continue;

			auto dist = vit_pos.distance(champ->Position());
			if (dist < temp_dist) {
				vit.owner = champ;
				temp_dist = dist;
			}
			
		}

		if (vit.type == VitalType::normal) {
			vit.timeout = time + 13250.0f;
			AddVital(vit, normal_vitals);
		}

		else if (vit.type == VitalType::warning) {
			vit.timeout = time + 1750.0f;
			AddVital(vit, arising_vitals);
		}

		else if (vit.type == VitalType::timeout)
			AddVital(vit, timingout_vitals);

		else if (vit.type == VitalType::ultimate) {
			vit.timeout = time + 8000.0f;
			AddVital(vit, ultimate_vitals);
		}

		return;
	}
}

/*
 * DeleteObject
 */
void DeleteObject(std::shared_ptr<GameObjectInstance> obj) {
	//auto name = obj->ObjectName();
	
	auto current_time = game->GameTime();
	auto found = std::find_if(normal_vitals.begin(), normal_vitals.end(), [&](const vital& elem) { return elem.networkId == obj->NetworkId() || elem.timeout < current_time; });
	if (found != normal_vitals.end()) {
		normal_vitals.erase(found);

	}

	found = std::find_if(arising_vitals.begin(), arising_vitals.end(), [&](const vital& elem) { return elem.networkId == obj->NetworkId() || elem.timeout < current_time; });
	if (found != arising_vitals.end()) {
		arising_vitals.erase(found);

	}

	found = std::find_if(timingout_vitals.begin(), timingout_vitals.end(), [&](const vital& elem) { return elem.networkId == obj->NetworkId() || elem.timeout < current_time; });
	if (found != timingout_vitals.end()) {
		timingout_vitals.erase(found);
	}

	found = std::find_if(ultimate_vitals.begin(), ultimate_vitals.end(), [&](const vital& elem) { return elem.networkId == obj->NetworkId() || elem.timeout < current_time; });
	if (found != ultimate_vitals.end()) {
		ultimate_vitals.erase(found);
	
	}

	//	auto found = std::find_if(arising_vitals.begin(), arising_vitals.end(), [&](const vital& elem) { return elem.networkId == obj->NetworkId(); });
	//	if (found != arising_vitals.end()) {
	//		arising_vitals.erase(found);
	//		return;
			//	std::cout << "Deleted Object: " << obj->ObjectName() << " _ " << obj->NetworkId() << std::endl;
	//	}
}

/*
 * GetBestVital
 */
bool GetBestVital(vital& _vital, std::vector<vital>& list, std::shared_ptr<GameObjectInstance> owner = nullptr) {
	
	float temp_distance = FLT_MAX;
	for (auto& vit : list) {

		if (owner != nullptr) {
			if (vit.owner == owner) {
				_vital = vit;
				return true;
			}
			continue;
		}

		if (vit.distance < temp_distance) {
			_vital = vit;
			temp_distance = vit.distance;
		}
	}

	if (list.size() > 0) {
		//_vital = list[0];
		return true;
	}
	return false;
}

/*
 * GetBestArisingVital
 */
bool GetBestArisingVital(vital& _vital) {
	if (arising_vitals.size() > 0) {
		_vital = arising_vitals[0];
		return true;
	}

	return false;
}

/*
 * RemoveVitals
 */
void RemoveVitals() {
	arising_vitals.erase(
		std::remove_if(
			arising_vitals.begin(),
			arising_vitals.end(),
			[](vital const& _vital) { return _vital.timeout <= game->GameTime(); }
		),
		arising_vitals.end()
	);
}

/*
 * Combo
 */
void Combo() {

	//myhero->ignoreEvade(true);

	static std::shared_ptr<GameObjectInstance> lat_vital_target;

	std::shared_ptr<GameObjectInstance> target;
/*	bool in_q_range = false;
	if (lat_vital_target != nullptr)
		in_q_range = myhero->inRange(lat_vital_target.get(), myhero->Q()->CastRange());

	if (lat_vital_target != nullptr && lat_vital_target->isValid() && in_q_range) {
		target = lat_vital_target;
	}
	else if(lat_vital_target == nullptr || !in_q_range){
		lat_vital_target = nullptr;*/
		target = targetselector->GetTarget(TargetSelectorTypes::health_absolute, myhero->Q()->CastRange() + 200.0f);
	//}
	
	if (target == nullptr) {
		orbwalker->SetOrbwalkingPoint(nullptr);
		return;
	}
		

	if (settings->combo.useq) {
		auto game_time = game->GameTime();
		vital vital;
		bool vital_exists = false;


		if (ultimate_vitals.size() > 0)
			vital_exists = GetBestVital(vital, ultimate_vitals, lat_vital_target);
		else if(normal_vitals.size() > 0)
			vital_exists = GetBestVital(vital, normal_vitals, lat_vital_target);
     	else
		vital_exists = GetBestVital(vital, arising_vitals, lat_vital_target);
	

		if (walk_vitals && vital_exists && myhero->Position().distance(vital.GetVitalPosition()) < 400.0f && (target->Position().direction(myhero->Position()).angel_between(target->Position().direction(vital.GetVitalPosition())) > 35.0f)) {
			auto vital_pos = vital.GetVitalPosition();
			//lat_vital_target = vital.owner;

             if (!predictionmanager->isWall(vital_pos)) {
				if (vital.direction == VitalDirection::NE)
					vital_pos = vital_pos.extend(target->Position(), -100.0f);

				orbwalker->SetOrbwalkingPoint(&vital_pos,true);

			}
		}
		else
			orbwalker->SetOrbwalkingPoint(nullptr);

		if (ultimate_vitals.size() > 0)
			vital_exists = GetBestVital(vital, ultimate_vitals, lat_vital_target);
		else
			vital_exists = GetBestVital(vital, normal_vitals, lat_vital_target);

		auto current_angle = target->Position().direction(myhero->Position()).angel_between(target->Position().direction(vital.GetVitalPosition()));
		auto myhero__in_range = myhero->inRange(target.get(), myhero->AttackRange());

		if (myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && vital_exists && myhero->Position().distance(vital.GetVitalPosition()) < myhero->Q()->CastRange() && ((current_angle > 35.0f || (current_angle <= 35.0f && !myhero__in_range)) || ultimate_vitals.size() > 0)) {
			if (!predictionmanager->isWall(vital.GetVitalPosition()))
				myhero->Q()->Cast(vital.GetVitalPosition());
		}
		else if (myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts() && !walk_vitals && myhero->inRange(target.get(), myhero->Q()->CastRange())) {

			myhero->Q()->Cast(target);
		}
	}


	if (settings->combo.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->R()->CastRange())) {
		myhero->R()->Cast(target);
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

	if (target == nullptr || (myhero->last_target != nullptr && myhero->last_target->NetworkId() == target->NetworkId()) || (myhero->CanAttack() && myhero->inRange(target.get(),myhero->AttackRange())))
		return;

	if (settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {
		if (myhero->Q()->AimTtype() != SkillAimType::activate)
			myhero->Q()->Cast(target);
		else
			myhero->Q()->Cast();
	}
	else if (settings->laneclear.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		if (myhero->E()->AimTtype() != SkillAimType::activate)
			myhero->E()->Cast(target);
		else
			myhero->E()->Cast();
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
		if (myhero->Q()->AimTtype() != SkillAimType::activate)
			myhero->Q()->Cast(target);
		else
			myhero->Q()->Cast();
	}
	else if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts() 
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		if (myhero->E()->AimTtype() != SkillAimType::activate)
			myhero->E()->Cast(target);
		else
			myhero->E()->Cast();
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

		if (draw_vital_position) {
			for (auto& vital : arising_vitals) {
				drawmanager->DrawCircle3D(vital.GetVitalPosition(), 200, false, 20, RGBA_COLOR(0, 222, 120, 255), 2);
			}
		}

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
 * setupMenu
 */
void setupMenu() {

	for (auto& champ : targetselector->m_all_heros) {

		if (champ->isAlly())
			continue;

		menu_spell_block.push_back({ champ->NetworkId(), champ->ChampionName() });
	}

	auto root_node = menu->CreateSubmenu("CoreFiora");

	auto sub_combo = menu->CreateSubmenu("Combo");
	auto sub_combo_w = menu->CreateTreeNode("Use W");
	for (auto& champ : menu_spell_block) {
		auto sub_combo_w_spells = menu->CreateSubmenu(champ._name);
		sub_combo_w->addItem(sub_combo_w_spells);
		sub_combo_w_spells->AddCheckbox("on Q", &champ.on_q);
		sub_combo_w_spells->AddCheckbox("on W", &champ.on_w);
		sub_combo_w_spells->AddCheckbox("on E", &champ.on_e);
		sub_combo_w_spells->AddCheckbox("on R", &champ.on_r);

	}
	
	sub_combo->AddCheckbox("Use Q", &settings->combo.useq);
	sub_combo->AddCheckbox("Orbwalk Vitals", &walk_vitals);

	sub_combo->AddCheckbox("Use W", &settings->combo.usew);

	sub_combo->addItem(sub_combo_w);
	sub_combo->AddCheckbox("Use E", &settings->combo.usee);
	sub_combo->AddCheckbox("Use R", &settings->combo.user);

	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	sub_laneclear->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use Q", &settings->lasthit.useq);
	sub_lasthit->AddCheckbox("Use E", &settings->lasthit.usee);

	auto sub_flee = menu->CreateSubmenu("Flee");
	sub_flee->AddCheckbox("Use Q to flee", &use_q_on_flee);

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Vital Positions", &draw_vital_position);
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);


	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	walk_vitals = settings->LoadCustomBool("Orbwalk Vitals");

	setupMenu();

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::activate);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	event_handler<Events::onCreateObject>::add_callback(CreateObject);
	event_handler<Events::onDeleteObject>::add_callback(DeleteObject);

	event_handler<Events::onUpdate>::add_callback(Update);

	event_handler<Events::onBeforeAttack>::add_callback(BeforeAttack);
	event_handler<Events::onAfterAttack>::add_callback(AfterAttack);
	event_handler<Events::onSpellCast>::add_callback(SpellCast);
	event_handler<Events::onActiveSpellCast>::add_callback(ActiveSpell);

	event_handler<Events::onMissileCreate>::add_callback(MissileCreate);
	event_handler<Events::onMissileRemoved>::add_callback(MissileRemove);

	event_handler<Events::onGainBuff>::add_callback(BuffGain);
	event_handler<Events::onLoseBuff>::add_callback(BuffLose);

	event_handler<Events::onGapCloser>::add_callback(GapCloser);

	event_handler<Events::onDraw>::add_callback(Draw);
}


void onUnload() {

	menu_spell_block.clear();

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