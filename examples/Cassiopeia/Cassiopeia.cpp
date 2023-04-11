#include "pch.h"
using namespace sdk;

static std::string championName = "Cassiopeia";
static std::string championVersion = "0.0.1";

CHAMPIONS(championName.c_str());
PLUGIN_NAME("CoreCassiopeia");
PLUGIN_TYPE(PluginTypes::CHAMPION);

// Default implementation
static std::shared_ptr<BuffInstance> buff;
static bool debug = false;

// Draw Toggle
static bool draw_q_range = true;
static bool draw_w_range = true;
static bool draw_e_range = true;
static bool draw_r_range = true;

static bool e_only_poisoned = true;
static bool q_only_not_poisoned = false;
static int minimum_r_targets = 1;
static int minimum_w_targets = 3;
static int minimum_q_targets = 2;
//damage
static float e_bonus_dmg[5] = {20.0f,40.0f,60.0f,80.0f,100.0f};


struct RectAngle  {

	const Vector3&  start;
	const Vector3&  end;

};

float E_damage(std::shared_ptr<GameObjectInstance> target) {

	auto current_ap = myhero->AbilityPower();
	auto e_base_dmg = 48.0f + 4.0f * myhero->Level() + (current_ap * 0.1f);
	float bonus = 0.0f;
	

	auto q_poison_buff = target->hasBuff(buff, "cassiopeiaqdebuff");
	auto w_poison_buff = target->hasBuff(buff, "cassiopeiawpoison");
	if (q_poison_buff || w_poison_buff)
		bonus = e_bonus_dmg[myhero->E()->Level() - 1] + (0.6 * current_ap);


	auto dmg = myhero->TotalOverallDamage(target.get(), 0.0f, (e_base_dmg + bonus));


	return dmg;
}

std::shared_ptr<GameObjectInstance> Get_E_LaneClear_Target() {

	std::shared_ptr<GameObjectInstance> highest_health_target = nullptr;
	float temphealth = FLT_MAX;

	for (auto& target : targetselector->m_all_units) {


		if (!target.second->isValid() || target.second->isAlly() || !myhero->inRange(target.second.get(), myhero->E()->CastRange()))
			continue;

		auto e_dmg = E_damage(target.second);
		auto health = target.second->PredictedHealth(myhero->E()->MissileSpeed(), myhero->E()->CastDelay());


	
	
	/*	if (health > e_dmg && health < temphealth) {
			temphealth = health;
			highest_health_target = target.second;
		}*/

		if (health <= e_dmg ) {
			return target.second;

		}

	}

//	if (highest_health_target != nullptr)
		//return highest_health_target;

	return nullptr;
}


struct RectangularPolygon {

	const Vector3& start;
	const Vector3& end;

};

RectangularPolygon& GetRectangle(const Vector3 & from, const Vector3& to, const float& width)  {

	auto direction = to.direction(from);
	auto perp_left = direction.perpendicular().normalize();
	auto perp_right = direction.perpendicular(true).normalize();


	auto point_on_left = to.add(perp_left * 400.0f);
	auto point_on_right = to.add(perp_right * 400.0f);

	auto rect = RectangularPolygon({ point_on_left ,point_on_right });
	return rect;
}

Vector3 GetWCastPosition()  {

	int max_count = 0;
	Vector3 cast_position;
	for (auto& target : targetselector->m_all_units) {


		if (!target.second->isValid() || target.second->isAlly() || !myhero->inRange(target.second.get(), myhero->W()->CastRange()))
			continue;

		int count = 0;
		auto pos = target.second->Position();
		auto rect = GetRectangle(myhero->Position(), pos, 160.0f);

		for (auto& target2 : targetselector->m_all_units) {


			if (!target2.second->isValid() || target2.second->isAlly() || target.second->NetworkId() == target2.second->NetworkId() ||  !myhero->inRange(target2.second.get(), myhero->W()->CastRange()))
				continue;


			if (predictionmanager->isPointInRectangle(rect.start, rect.end, 160.0f, target2.second->Position())) {
				count++;

			}

		}

		if (count > max_count) {
			max_count = count;
			cast_position = pos;
		
		}

	}

	if (max_count < minimum_w_targets)
		return Vector3();

	return cast_position;
}

/*
 * BeforeAttack
 */
void BeforeAttack(GameObjectInstance* target, bool* process) {
	if (target == nullptr)
		return;

/*	if (settings->laneclear.usee && orbwalker->Mode() == OrbwalkModes::LANECLEAR) {
		auto e_target = Get_E_LaneClear_Target();
		if (e_target != nullptr && (myhero->E()->isReady() || (game->GameTime() - myhero->E()->SpellReadyTime()) < myhero->GetWindup()))
			*process = false;
	}*/
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
	// q = asheqcastready
	//	std::cout << "Buff Name: " << buff->Name() << std::endl;
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




Vector3 GetRCastPosition() {

	std::vector< std::shared_ptr<GameObjectInstance>> facing_targets;
	std::vector<Vector3>  found_targets;
	auto myhero_pos = myhero->Position();
	for (auto& target : targetselector->m_all_heros) {

		if (!target->isValid() || target->isAlly() || !myhero->inRange(target.get(), myhero->R()->CastRange()-100.0f) || !target->isFacing(myhero) || myhero_pos.distance(target->Position()) < 200.0f )
			continue;

		facing_targets.push_back(target);
	}

	auto facing_count = facing_targets.size();

	if (facing_count == 0)
		return Vector3();

	else if (facing_count == 1) {
		if (minimum_r_targets == 1)
			return facing_targets[0]->Position().direction(myhero_pos).normalize();
		else
			return Vector3();
	}
		
	if(minimum_r_targets == 1)
		return facing_targets[0]->Position().direction(myhero_pos).normalize();

	int max_count = 0;
	for (auto& target : facing_targets) {

		auto target_pos = target->Position();
		std::vector<Vector3> facing_positions;
		facing_positions.push_back(target_pos.direction(myhero_pos).normalize());
		int count = 0;

		for (auto& target_second : facing_targets) {

			if (target->NetworkId() == target_second->NetworkId())
				continue;
			auto second_target_pos = target_second->Position();
			bool between = predictionmanager->isBetween(myhero_pos, target_pos, second_target_pos, 60.0f);
			
			if (between) {
				facing_positions.push_back(second_target_pos.direction(myhero_pos).normalize());
				count++;
			}
		}

		if (count > max_count ) {
			max_count = count;
			found_targets = facing_positions;
		}

	}

	if (found_targets.size() < minimum_r_targets)
		return Vector3();

	Vector3 center_pos;
	for (auto& final_trgets : found_targets) {

		if (center_pos.x == 0.0 && center_pos.y == 0.0 && center_pos.z == 0.0)
			center_pos = final_trgets;
		else
			center_pos = center_pos.add(final_trgets);

	}



	return center_pos.normalize();
}


/*
 * Combo
 */
void Combo() {

	auto target = targetselector->GetTarget(sdk::TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr)
		return;


	auto facing_angle = target->isFacing(myhero);

	auto q_poison_buff = target->hasBuff(buff, "cassiopeiaqdebuff");
	auto w_poison_buff = target->hasBuff(buff, "cassiopeiawpoison");


	if (settings->combo.useq && (!q_only_not_poisoned || !q_poison_buff) && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->Q()->CastRange())) {

		auto pos = predictionmanager->GetPredictedPositionTimeBased(myhero.get(), target.get(), 0.9f, myhero->Q()->CastDelay() / 1000.0f);

		if(pos.isValid() && myhero->Position().distance(pos) < myhero->Q()->CastRange())
		myhero->Q()->Cast(pos);
	}
	
	if (settings->combo.usew && !w_poison_buff && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->W()->CastRange())) {
		myhero->W()->Cast(target);
	}
	
	if (settings->combo.usee && (!e_only_poisoned || q_poison_buff || w_poison_buff) && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->E()->CastRange())) {
		myhero->E()->Cast(target);
	}
	
	if (settings->combo.user && myhero->R()->isReady() && myhero->CurrentMana() >= myhero->R()->GetManaCosts()
		&& myhero->inRange(target.get(), myhero->R()->CastRange())) {

		auto R_targets = GetRCastPosition();

		if (R_targets.x != 0.0 || R_targets.y != 0.0 || R_targets.z != 0.0) {
			auto pos = myhero->Position().add(R_targets * 600.0f);
			myhero->R()->Cast(pos);
		}
	}
}
/*
 * LastHit
 */
void LastHit() {

	if (settings->lasthit.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()) {
		auto target = Get_E_LaneClear_Target();
		if (target != nullptr)
			myhero->E()->Cast(target);
	}
}
/*
 * Herass
 */
void Herass() {

	LastHit();

	
}


Vector3 GetQTarget() {

	int max_count = 0;
	Vector3 cast_position;
	for (auto& target : targetselector->m_all_units) {


		if (!target.second->isValid() || target.second->isAlly() || !myhero->inRange(target.second.get(), myhero->Q()->CastRange()))
			continue;

		int count = 0;
		auto pos = target.second->Position();

		for (auto& target2 : targetselector->m_all_units) {


			if (!target2.second->isValid() || target2.second->isAlly() || target.second->NetworkId() == target2.second->NetworkId() || !myhero->inRange(target2.second.get(), myhero->Q()->CastRange()+200.0f))
				continue;


			if (pos.distance(target2.second->Position()) <= 250.0f) {
				count++;

			}

		}

		if (count > max_count) {
			max_count = count;
			cast_position = pos;

		}

	}

	if (max_count < minimum_q_targets)
		return Vector3();

	return cast_position;

}


/*
 * LaneClear
 */
void LaneClear() {


	if(myhero->Level() < 10 && (myhero->CurrentMana() / myhero->MaxMana() < 0.4))
		return;

	bool casted = false;

	if (settings->laneclear.usee && myhero->E()->isReady() && myhero->CurrentMana() >= myhero->E()->GetManaCosts()) {
		auto target = Get_E_LaneClear_Target();

		if (target != nullptr) {
			myhero->E()->Cast(target);
			casted = true;
		}
	}
	
	if (!casted && settings->laneclear.useq && myhero->Q()->isReady() && myhero->CurrentMana() >= myhero->Q()->GetManaCosts()) {
	
		auto target = GetQTarget();

		if (!target.isValid())
			return;

     	myhero->Q()->Cast(target);
		
		casted = true;
	}

	if (!casted && settings->laneclear.usew && myhero->W()->isReady() && myhero->CurrentMana() >= myhero->W()->GetManaCosts()) {
			auto target_pos = GetWCastPosition();

			if (target_pos.isValid()) {
				myhero->W()->Cast(target_pos);
				casted = true;
			}
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
	else if (orbwalker->Mode() == OrbwalkModes::NONE) {
		// TODO add E to jungle camps or jungler last position  when E uts bit visible and its not visible
	}
}


/*
 * Draw
 */
void Draw() {


	if (!myhero->isAlive())
		return;


	/*auto mouse = game->GetMousePosition();

	auto direction = mouse.direction(myhero->Position());// .distance(mouse);
	auto perp_left = direction.perpendicular().normalize();
	auto perp_right = direction.perpendicular(true).normalize();


	auto point_on_left = mouse.add(perp_left * 400.0f);
	auto point_on_right = mouse.add(perp_right * 400.0f);



	drawmanager->DrawRect3D(point_on_left, point_on_right, 160.0f, RGBA_COLOR(135, 0, 100, 255));*/

	

	if (draw_q_range && myhero->Q()->Level() > 0) {
		drawmanager->DrawCircle3D(myhero->Position(), myhero->Q()->CastRange(), false, 60, RGBA_COLOR(135, 0, 100, 255), 5.0);
	}

	if (draw_w_range && myhero->W()->Level() > 0) {
		drawmanager->DrawCircle3D(myhero->Position(), myhero->W()->CastRange(), false, 60, RGBA_COLOR(140, 142, 100, 255), 5.0);
	}

	if (draw_e_range && myhero->E()->Level() > 0) {
		drawmanager->DrawCircle3D(myhero->Position(), myhero->E()->CastRange(), false, 60, RGBA_COLOR(78, 142, 100, 255), 5.0);
	}

	if (draw_r_range && myhero->R()->Level() > 0) {
		drawmanager->DrawCircle3D(myhero->Position(), myhero->R()->CastRange(), false, 60, RGBA_COLOR(150, 100, 100, 255), 5.0);
	}


	auto target = targetselector->GetTarget(sdk::TargetSelectorTypes::health_absolute, 1000.0f);

	if (target == nullptr)
		return;
	
	//if (predictionmanager->isPointInRectangle(point_on_left, point_on_right, 160.0f, target->Position()))
	//	drawmanager->DrawCircle3D(mouse, 20, true, 20, RGBA_COLOR(135, 0, 100, 255), 2);

	auto pos = predictionmanager->GetPredictedPositionTimeBased(myhero.get(), target.get(), 0.9f, myhero->Q()->CastDelay() / 1000.0f);

	if (pos.isValid())
		drawmanager->DrawCircle3D(pos, 20, true, 20, RGBA_COLOR(135, 0, 100, 255), 2);

}

/*
 * Menu
 */
void Menu() {

	auto root_node = menu->CreateSubmenu("CoreCassiopeia");

	auto drawing_node = menu->CreateSubmenu("Drawings");
	drawing_node->AddCheckbox("Draw Q Range", &draw_q_range, {}, true);
	drawing_node->AddCheckbox("Draw W Range", &draw_w_range, {}, true);
	drawing_node->AddCheckbox("Draw E Range", &draw_e_range, {}, true);
	drawing_node->AddCheckbox("Draw R Range", &draw_r_range, {}, true);


	auto sub_combo = menu->CreateSubmenu("Combo");
	auto sub_q = menu->CreateSubmenu("Q");
	auto sub_w = menu->CreateSubmenu("W");
	auto sub_e = menu->CreateSubmenu("E");
	auto sub_r = menu->CreateSubmenu("R");
	sub_q->AddCheckbox("Use Q", &settings->combo.useq);
	sub_q->AddCheckbox("Only not poisoned", &q_only_not_poisoned);

	sub_w->AddCheckbox("Use W", &settings->combo.usew);


	sub_e->AddCheckbox("Use E", &settings->combo.usee);
	sub_e->AddCheckbox("Only on poisoned", &e_only_poisoned);

	sub_r->AddCheckbox("Use R", &settings->combo.user);
	sub_r->AddSliderInt("Min R Targets", minimum_r_targets, 1, 5, {}, true);



	sub_combo->addItems({ sub_q ,sub_w ,sub_e ,sub_r });


	auto sub_herass = menu->CreateSubmenu("Herass");
	sub_herass->AddCheckbox("Use Q", &settings->herass.useq);
	sub_herass->AddCheckbox("Use W", &settings->herass.usew);
	sub_herass->AddCheckbox("Use E", &settings->herass.usee);

	auto sub_laneclear = menu->CreateSubmenu("LaneClear");
	auto sub_laneclear_q = menu->CreateSubmenu("Q");
	auto sub_laneclear_w = menu->CreateSubmenu("W");

	sub_laneclear_q->AddCheckbox("Use Q", &settings->laneclear.useq);
	sub_laneclear_q->AddSliderInt("Min Q Targets", minimum_q_targets, 1, 5, {}, true);

	sub_laneclear_w->AddCheckbox("Use W", &settings->laneclear.usew);
	sub_laneclear_w->AddSliderInt("Min W Targets", minimum_w_targets, 1, 5, {}, true);

	sub_laneclear->addItems({ sub_laneclear_q ,sub_laneclear_w  });

	sub_laneclear->AddCheckbox("Use E", &settings->laneclear.usee);

	auto sub_lasthit = menu->CreateSubmenu("LastHit");
	sub_lasthit->AddCheckbox("Use W", &settings->lasthit.usew);


	root_node->addItems({ sub_combo, sub_herass, sub_laneclear, sub_lasthit, drawing_node });

	menu->addItem(root_node);
}

/*
 * onLoad
 */
void onLoad() {
	std::cout << " [LOG]: " << championName << " init version: " << championVersion << " Loaded" << std::endl;

	myhero->Q()->RegisterSpell(SkillAimType::direction_to_target,0.25f);
	myhero->W()->RegisterSpell(SkillAimType::direction_to_target);
	myhero->E()->RegisterSpell(SkillAimType::direction_to_target, 0.16f);
	myhero->R()->RegisterSpell(SkillAimType::direction_to_target);

	myhero->SUM1()->RegisterSpell(SkillAimType::activate);
	myhero->SUM2()->RegisterSpell(SkillAimType::activate);

	q_only_not_poisoned = settings->LoadCustomBool("Only not poisoned");
	draw_q_range = settings->LoadCustomBool("Draw Q Range");
	draw_w_range = settings->LoadCustomBool("Draw W Range");
	draw_e_range = settings->LoadCustomBool("Draw E Range");
	draw_r_range = settings->LoadCustomBool("Draw R Range");

	minimum_q_targets = settings->LoadCustomInt("Min Q Targets") == 0 ? minimum_q_targets : settings->LoadCustomInt("Min Q Targets");
	minimum_w_targets = settings->LoadCustomInt("Min Q Targets") == 0 ? minimum_w_targets : settings->LoadCustomInt("Min W Targets");
	minimum_r_targets = settings->LoadCustomInt("Min Q Targets") == 0 ? minimum_r_targets : settings->LoadCustomInt("Min R Targets");


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