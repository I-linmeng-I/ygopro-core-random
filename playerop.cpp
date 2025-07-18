/*
 * playerop.cpp
 *
 *  Created on: 2010-12-22
 *      Author: Argon
 */

#include "field.h"
#include "duel.h"
#include "effect.h"
#include "card.h"
#include "ocgapi.h"

#include <algorithm>
#include <stack>

bool field::check_response(size_t vector_size, int32_t min_len, int32_t max_len) const {
	const int32_t len = returns.bvalue[0];
	if (len < min_len || len > max_len)
		return false;
	std::set<uint8_t> index_set;
	for (int32_t i = 0; i < len; ++i) {
		uint8_t index = returns.bvalue[1 + i];
		if (index >= (int32_t)vector_size  || index_set.count(index)) {
			return false;
		}
		index_set.insert(index);
	}
	return true;
}
int32_t field::select_battle_command(uint16_t step, uint8_t playerid) {
	if(step == 0) {
		pduel->write_buffer8(MSG_SELECT_BATTLECMD);
		pduel->write_buffer8(playerid);
		//Activatable
		pduel->write_buffer8((uint8_t)core.select_chains.size());
		std::sort(core.select_chains.begin(), core.select_chains.end(), chain::chain_operation_sort);
		for(const auto& ch : core.select_chains) {
			effect* peffect = ch.triggering_effect;
			card* pcard = peffect->get_handler();
			if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY))
				pduel->write_buffer32(pcard->data.code);
			else
				pduel->write_buffer32(pcard->data.code | 0x80000000);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
			pduel->write_buffer32(peffect->description);
		}
		//Attackable
		pduel->write_buffer8((uint8_t)core.attackable_cards.size());
		for(auto& pcard : core.attackable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
			pduel->write_buffer8(pcard->direct_attackable);
		}
		//M2, EP
		if(core.to_m2)
			pduel->write_buffer8(1);
		else
			pduel->write_buffer8(0);
		if(core.to_ep)
			pduel->write_buffer8(1);
		else
			pduel->write_buffer8(0);
		return FALSE;
	} else {
		int32_t t = (uint32_t)returns.ivalue[0] & 0xffff;
		int32_t s = (uint32_t)returns.ivalue[0] >> 16;
		if(t < 0 || t > 3 || s < 0
		        || (t == 0 && s >= (int32_t)core.select_chains.size())
		        || (t == 1 && s >= (int32_t)core.attackable_cards.size())
		        || (t == 2 && !core.to_m2)
		        || (t == 3 && !core.to_ep)) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_idle_command(uint16_t step, uint8_t playerid) {
	if(step == 0) {
		pduel->write_buffer8(MSG_SELECT_IDLECMD);
		pduel->write_buffer8(playerid);
		//idle summon
		pduel->write_buffer8((uint8_t)core.summonable_cards.size());
		for(auto& pcard : core.summonable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		//idle spsummon
		pduel->write_buffer8((uint8_t)core.spsummonable_cards.size());
		for(auto& pcard : core.spsummonable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		//idle pos change
		pduel->write_buffer8((uint8_t)core.repositionable_cards.size());
		for(auto& pcard : core.repositionable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		//idle mset
		pduel->write_buffer8((uint8_t)core.msetable_cards.size());
		for(auto& pcard : core.msetable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		//idle sset
		pduel->write_buffer8((uint8_t)core.ssetable_cards.size());
		for(auto& pcard : core.ssetable_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		//idle activate
		pduel->write_buffer8((uint8_t)core.select_chains.size());
		std::sort(core.select_chains.begin(), core.select_chains.end(), chain::chain_operation_sort);
		for(const auto& ch : core.select_chains) {
			effect* peffect = ch.triggering_effect;
			card* pcard = peffect->get_handler();
			if(!peffect->is_flag(EFFECT_FLAG_FIELD_ONLY))
				pduel->write_buffer32(pcard->data.code);
			else
				pduel->write_buffer32(pcard->data.code | 0x80000000);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
			pduel->write_buffer32(peffect->description);
		}
		//To BP
		if(infos.phase == PHASE_MAIN1 && core.to_bp)
			pduel->write_buffer8(1);
		else
			pduel->write_buffer8(0);
		if(core.to_ep)
			pduel->write_buffer8(1);
		else
			pduel->write_buffer8(0);
		if(infos.can_shuffle && player[playerid].list_hand.size() > 1)
			pduel->write_buffer8(1);
		else
			pduel->write_buffer8(0);
		return FALSE;
	} else {
		int32_t t = (uint32_t)returns.ivalue[0] & 0xffff;
		int32_t s = (uint32_t)returns.ivalue[0] >> 16;
		if(t < 0 || t > 8 || s < 0
		        || (t == 0 && s >= (int32_t)core.summonable_cards.size())
		        || (t == 1 && s >= (int32_t)core.spsummonable_cards.size())
		        || (t == 2 && s >= (int32_t)core.repositionable_cards.size())
		        || (t == 3 && s >= (int32_t)core.msetable_cards.size())
		        || (t == 4 && s >= (int32_t)core.ssetable_cards.size())
		        || (t == 5 && s >= (int32_t)core.select_chains.size())
		        || (t == 6 && (infos.phase != PHASE_MAIN1 || !core.to_bp))
		        || (t == 7 && !core.to_ep)
		        || (t == 8 && !(infos.can_shuffle && (int32_t)player[playerid].list_hand.size() > 1))) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_effect_yes_no(uint16_t step, uint8_t playerid, uint32_t description, card* pcard) {
	if(step == 0) {
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.ivalue[0] = 1;
			return TRUE;
		}
		pduel->write_buffer8(MSG_SELECT_EFFECTYN);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(pcard->data.code);
		pduel->write_buffer32(pcard->get_info_location());
		pduel->write_buffer32(description);
		returns.ivalue[0] = -1;
		return FALSE;
	} else {
		if(returns.ivalue[0] != 0 && returns.ivalue[0] != 1) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_yes_no(uint16_t step, uint8_t playerid, uint32_t description) {
	if(step == 0) {
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.ivalue[0] = 1;
			return TRUE;
		}
		pduel->write_buffer8(MSG_SELECT_YESNO);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(description);
		returns.ivalue[0] = -1;
		return FALSE;
	} else {
		if(returns.ivalue[0] != 0 && returns.ivalue[0] != 1) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_option(uint16_t step, uint8_t playerid) {
	if(step == 0) {
		returns.ivalue[0] = -1;
		if(core.select_options.size() == 0)
			return TRUE;
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.ivalue[0] = 0;
			return TRUE;
		}
		pduel->write_buffer8(MSG_SELECT_OPTION);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8((uint8_t)core.select_options.size());
		for(auto& option : core.select_options)
			pduel->write_buffer32(option);
		return FALSE;
	} else {
		if(returns.ivalue[0] < 0 || returns.ivalue[0] >= (int32_t)core.select_options.size()) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_card(uint16_t step, uint8_t playerid, uint8_t cancelable, uint8_t min, uint8_t max) {
	if(step == 0) {
		returns.bvalue[0] = 0;
		if(max == 0 || core.select_cards.empty())
			return TRUE;
		core.selecting_player = playerid;
		std::sort(core.select_cards.begin(), core.select_cards.end(), card::card_operation_sort);
		if (core.select_cards.size() > UINT8_MAX)
			core.select_cards.resize(UINT8_MAX);
		if(max > core.select_cards.size())
			max = (uint8_t)core.select_cards.size();
		if(min > max)
			min = max;
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.bvalue[0] = min;
			for(uint8_t i = 0; i < min; ++i)
				returns.bvalue[i + 1] = i;
			return TRUE;
		}
		core.units.begin()->arg2 = ((uint32_t)min) + (((uint32_t)max) << 16);
		pduel->write_buffer8(MSG_SELECT_CARD);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(cancelable);
		pduel->write_buffer8(min);
		pduel->write_buffer8(max);
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		uint8_t deck_seq = 0;
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer32(pcard->get_select_info_location(&deck_seq));
		}
		return FALSE;
	} else {
		if (returns.ivalue[0] == -1) {
			if (cancelable)
				return TRUE;
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		if (!check_response(core.select_cards.size(), min, max)) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_unselect_card(uint16_t step, uint8_t playerid, uint8_t cancelable, uint8_t min, uint8_t max, uint8_t finishable) {
	if(step == 0) {
		returns.bvalue[0] = 0;
		if(core.select_cards.empty() && core.unselect_cards.empty())
			return TRUE;
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.bvalue[0] = 1;
			for(uint8_t i = 0; i < 1; ++i)
				returns.bvalue[i + 1] = i;
			return TRUE;
		}
		core.selecting_player = playerid;
		std::sort(core.select_cards.begin(), core.select_cards.end(), card::card_operation_sort);
		if (core.select_cards.size() > UINT8_MAX)
			core.select_cards.resize(UINT8_MAX);
		if (core.unselect_cards.size() > UINT8_MAX)
			core.unselect_cards.resize(UINT8_MAX);
		pduel->write_buffer8(MSG_SELECT_UNSELECT_CARD);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(finishable);
		pduel->write_buffer8(cancelable);
		pduel->write_buffer8(min);
		pduel->write_buffer8(max);
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		uint8_t deck_seq = 0;
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer32(pcard->get_select_info_location(&deck_seq));
		}
		pduel->write_buffer8((uint8_t)core.unselect_cards.size());
		for(auto& pcard : core.unselect_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer32(pcard->get_select_info_location(&deck_seq));
		}
		return FALSE;
	} else {
		if(returns.ivalue[0] == -1) {
			if(cancelable || finishable)
				return TRUE;
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		if (!check_response(core.select_cards.size() + core.unselect_cards.size(), 1, 1)) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_chain(uint16_t step, uint8_t playerid, uint8_t spe_count) {
	if(step == 0) {
		returns.ivalue[0] = -1;
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			if(core.select_chains.size() == 0)
				returns.ivalue[0] = -1;
			else {
				bool act = true;
				for(const auto& ch : core.current_chain)
					if(ch.triggering_player == 1)
						act = false;
				if(act)
					returns.ivalue[0] = 0;
				else
					returns.ivalue[0] = -1;
			}
			return TRUE;
		}
		pduel->write_buffer8(MSG_SELECT_CHAIN);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8((uint8_t)core.select_chains.size());
		pduel->write_buffer8(spe_count);
		pduel->write_buffer32(pduel->game_field->core.hint_timing[playerid]);
		pduel->write_buffer32(pduel->game_field->core.hint_timing[1 - playerid]);
		std::sort(core.select_chains.begin(), core.select_chains.end(), chain::chain_operation_sort);
		for(const auto& ch : core.select_chains) {
			effect* peffect = ch.triggering_effect;
			card* pcard = peffect->get_handler();
			if(peffect->is_flag(EFFECT_FLAG_FIELD_ONLY))
				pduel->write_buffer8(EDESC_OPERATION);
			else if(!(peffect->type & EFFECT_TYPE_ACTIONS))
				pduel->write_buffer8(EDESC_RESET);
			else
				pduel->write_buffer8(0);
			if(ch.flag & CHAIN_FORCED)
				pduel->write_buffer8(1);
			else
				pduel->write_buffer8(0);
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer32(pcard->get_info_location());
			pduel->write_buffer32(peffect->description);
		}
		return FALSE;
	} else {
		if(returns.ivalue[0] == -1) {
			for(const auto& ch : core.select_chains) {
				if(ch.flag & CHAIN_FORCED) {
					pduel->write_buffer8(MSG_RETRY);
					return FALSE;
				}
			}
			return TRUE;
		}
		if(returns.ivalue[0] < 0 || returns.ivalue[0] >= (int32_t)core.select_chains.size()) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_place(uint16_t step, uint8_t playerid, uint32_t flag, uint8_t count) {
	if(step == 0) {
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			flag = ~flag;
			uint32_t filter;
			int32_t pzone = 0;
			if(flag & 0x7f) {
				returns.bvalue[0] = 1;
				returns.bvalue[1] = LOCATION_MZONE;
				filter = flag & 0x7f;
			} else if(flag & 0x1f00) {
				returns.bvalue[0] = 1;
				returns.bvalue[1] = LOCATION_SZONE;
				filter = (flag >> 8) & 0x1f;
			} else if(flag & 0xc000) {
				returns.bvalue[0] = 1;
				returns.bvalue[1] = LOCATION_SZONE;
				filter = (flag >> 14) & 0x3;
				pzone = 1;
			} else if(flag & 0x7f0000) {
				returns.bvalue[0] = 0;
				returns.bvalue[1] = LOCATION_MZONE;
				filter = (flag >> 16) & 0x7f;
			} else if(flag & 0x1f000000) {
				returns.bvalue[0] = 0;
				returns.bvalue[1] = LOCATION_SZONE;
				filter = (flag >> 24) & 0x1f;
			} else {
				returns.bvalue[0] = 0;
				returns.bvalue[1] = LOCATION_SZONE;
				filter = (flag >> 30) & 0x3;
				pzone = 1;
			}
			if(!pzone) {
				if(filter & 0x40) returns.bvalue[2] = 6;
				else if(filter & 0x20) returns.bvalue[2] = 5;
				else if(filter & 0x4) returns.bvalue[2] = 2;
				else if(filter & 0x2) returns.bvalue[2] = 1;
				else if(filter & 0x8) returns.bvalue[2] = 3;
				else if(filter & 0x1) returns.bvalue[2] = 0;
				else if(filter & 0x10) returns.bvalue[2] = 4;
			} else {
				if(filter & 0x1) returns.bvalue[2] = 6;
				else if(filter & 0x2) returns.bvalue[2] = 7;
			}
			return TRUE;
		}
		if(core.units.begin()->type == PROCESSOR_SELECT_PLACE)
			pduel->write_buffer8(MSG_SELECT_PLACE);
		else
			pduel->write_buffer8(MSG_SELECT_DISFIELD);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(count);
		pduel->write_buffer32(flag);
		returns.bvalue[0] = 0;
		return FALSE;
	} else {
		uint8_t pt = 0;
		uint32_t selected = 0;
		int32_t len = std::max(1, (int32_t)count);
		for (int32_t i = 0; i < len; ++i) {
			uint8_t p = returns.bvalue[pt];
			uint8_t l = returns.bvalue[pt + 1];
			uint8_t s = returns.bvalue[pt + 2];
			uint32_t sel = 0x1u << (s + (p == playerid ? 0 : 16) + (l == LOCATION_MZONE ? 0 : 8));
			if(!(count == 0 && i == 0 && l == 0)
				&& ((p != 0 && p != 1)
					|| ((l != LOCATION_MZONE) && (l != LOCATION_SZONE))
					|| (sel & flag) || (sel & selected))) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			if(sel & (0x1u << 5))
				sel |= 0x1u << (16 + 6);
			if(sel & (0x1u << 6))
				sel |= 0x1u << (16 + 5);
			selected |= sel;
			pt += 3;
		}
		return TRUE;
	}
}
int32_t field::select_position(uint16_t step, uint8_t playerid, uint32_t code, uint8_t positions) {
	if(step == 0) {
		if(positions == 0) {
			returns.ivalue[0] = POS_FACEUP_ATTACK;
			return TRUE;
		}
		positions &= 0xf;
		if(positions == 0x1 || positions == 0x2 || positions == 0x4 || positions == 0x8) {
			returns.ivalue[0] = positions;
			return TRUE;
		}
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			if((uint32_t)positions & 0x4)
				returns.ivalue[0] = 0x4;
			else if((uint32_t)positions & 0x1)
				returns.ivalue[0] = 0x1;
			else if((uint32_t)positions & 0x8)
				returns.ivalue[0] = 0x8;
			else
				returns.ivalue[0] = 0x2;
			return TRUE;
		}
		pduel->write_buffer8(MSG_SELECT_POSITION);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(code);
		pduel->write_buffer8(positions);
		returns.ivalue[0] = 0;
		return FALSE;
	} else {
		uint32_t pos = returns.ivalue[0];
		if(pos != 0x1 && pos != 0x2 && pos != 0x4 && pos != 0x8 || !(pos & positions)) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_tribute(uint16_t step, uint8_t playerid, uint8_t cancelable, uint8_t min, uint8_t max) {
	if(step == 0) {
		returns.bvalue[0] = 0;
		if(max == 0 || core.select_cards.empty())
			return TRUE;
		core.selecting_player = playerid;
		std::sort(core.select_cards.begin(), core.select_cards.end(), card::card_operation_sort);
		if (core.select_cards.size() > UINT8_MAX)
			core.select_cards.resize(UINT8_MAX);
		uint8_t tm = 0;
		for(auto& pcard : core.select_cards)
			tm += pcard->release_param;
		if(max > 5)
			max = 5;
		if(max > tm)
			max = tm;
		if(min > max)
			min = max;
		core.units.begin()->arg2 = ((uint32_t)min) + (((uint32_t)max) << 16);
		pduel->write_buffer8(MSG_SELECT_TRIBUTE);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(cancelable);
		pduel->write_buffer8(min);
		pduel->write_buffer8(max);
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		uint8_t deck_seq = 0;
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->get_select_sequence(&deck_seq));
			pduel->write_buffer8(pcard->release_param);
		}
		return FALSE;
	} else {
		if (returns.ivalue[0] == -1) {
			if (cancelable)
				return TRUE;
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		if(returns.bvalue[0] > max) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		std::set<uint8_t> c;
		int32_t m = (int32_t)core.select_cards.size(), tt = 0;
		for(int32_t i = 0; i < returns.bvalue[0]; ++i) {
			uint8_t v = returns.bvalue[i + 1];
			if(v >= m || c.count(v)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			c.insert(v);
			tt += core.select_cards[v]->release_param;
		}
		if(tt < min) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		return TRUE;
	}
}
int32_t field::select_counter(uint16_t step, uint8_t playerid, uint16_t countertype, uint16_t count, uint8_t s, uint8_t o) {
	if(step == 0) {
		if(count == 0)
			return TRUE;
		uint8_t avail = s;
		uint8_t fp = playerid;
		uint32_t total = 0;
		core.select_cards.clear();
		for(int p = 0; p < 2; ++p) {
			if(avail) {
				for(auto& pcard : player[fp].list_mzone) {
					if(pcard && pcard->get_counter(countertype)) {
						core.select_cards.push_back(pcard);
						total += pcard->get_counter(countertype);
					}
				}
				for(auto& pcard : player[fp].list_szone) {
					if(pcard && pcard->get_counter(countertype)) {
						core.select_cards.push_back(pcard);
						total += pcard->get_counter(countertype);
					}
				}
			}
			fp = 1 - fp;
			avail = o;
		}
		if(count > total) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		pduel->write_buffer8(MSG_SELECT_COUNTER);
		pduel->write_buffer8(playerid);
		pduel->write_buffer16(countertype);
		pduel->write_buffer16(count);
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		core.selecting_player = playerid;
		std::sort(core.select_cards.begin(), core.select_cards.end(), card::card_operation_sort);
		uint8_t deck_seq = 0;
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->get_select_sequence(&deck_seq));
			pduel->write_buffer16(pcard->get_counter(countertype));
		}
		return FALSE;
	} else {
		int32_t ct = 0;
		for(int32_t i = 0; i < (int32_t)core.select_cards.size(); ++i) {
			if(core.select_cards[i]->get_counter(countertype) < returns.svalue[i]) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			ct += returns.svalue[i];
		}
		if(ct != count) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
	}
	return TRUE;
}
static int32_t select_sum_check1(const uint32_t* oparam, int32_t size, int32_t index, int32_t acc, int32_t opmin) {
	if(acc == 0 || index == size)
		return FALSE;
	int32_t o1, o2;
	field::get_sum_params(oparam[index], o1, o2);
	if(index == size - 1)
		return (acc == o1 && acc + opmin > o1) || (o2 && acc == o2 && acc + opmin > o2);
	return (acc > o1 && select_sum_check1(oparam, size, index + 1, acc - o1, std::min(o1, opmin)))
	       || (o2 > 0 && acc > o2 && select_sum_check1(oparam, size, index + 1, acc - o2, std::min(o2, opmin)));
}
int32_t field::select_with_sum_limit(int16_t step, uint8_t playerid, int32_t acc, int32_t min, int32_t max) {
	if(step == 0) {
		returns.bvalue[0] = 0;
		if(core.select_cards.empty())
			return TRUE;
		core.selecting_player = playerid;
		std::sort(core.select_cards.begin(), core.select_cards.end(), card::card_operation_sort);
		if (core.select_cards.size() > UINT8_MAX)
			core.select_cards.resize(UINT8_MAX);
		if (core.must_select_cards.size() > UINT8_MAX)
			core.must_select_cards.resize(UINT8_MAX);
		pduel->write_buffer8(MSG_SELECT_SUM);
		if(max)
			pduel->write_buffer8(0);
		else
			pduel->write_buffer8(1);
		if(max < min)
			max = min;
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(acc);
		pduel->write_buffer8(min);
		pduel->write_buffer8(max);
		pduel->write_buffer8((uint8_t)core.must_select_cards.size());
		uint8_t deck_seq = 0;
		for(auto& pcard : core.must_select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->get_select_sequence(&deck_seq));
			pduel->write_buffer32(pcard->sum_param);
		}
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->get_select_sequence(&deck_seq));
			pduel->write_buffer32(pcard->sum_param);
		}
		return FALSE;
	} else {
		std::set<int32_t> c;
		int32_t mcount = (int32_t)core.must_select_cards.size();
		if (mcount > UINT8_MAX)
			mcount = UINT8_MAX;
		if(max) {
			uint32_t oparam[512]{};			
			if(returns.bvalue[0] < min + mcount || returns.bvalue[0] > max + mcount) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			for(int32_t i = 0; i < mcount; ++i)
				oparam[i] = core.must_select_cards[i]->sum_param;
			int32_t m = (int32_t)core.select_cards.size();
			for(int32_t i = mcount; i < returns.bvalue[0]; ++i) {
				int32_t v = returns.bvalue[i + 1];
				if(v < 0 || v >= m || c.count(v)) {
					pduel->write_buffer8(MSG_RETRY);
					return FALSE;
				}
				c.insert(v);
				oparam[i] = core.select_cards[v]->sum_param;
			}
			if(!select_sum_check1(oparam, returns.bvalue[0], 0, acc, 0xffff)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			return TRUE;
		} else {
			int32_t sum = 0, mx = 0, mn = 0x7fffffff;
			for(int32_t i = 0; i < mcount; ++i) {
				int32_t o1, o2;
				field::get_sum_params(core.must_select_cards[i]->sum_param, o1, o2);
				int32_t ms = (o2 && o2 < o1) ? o2 : o1;
				sum += ms;
				mx += std::max(o1, o2);
				if(ms < mn)
					mn = ms;
			}
			int32_t m = (int32_t)core.select_cards.size();
			for(int32_t i = mcount; i < returns.bvalue[0]; ++i) {
				int32_t v = returns.bvalue[i + 1];
				if(v < 0 || v >= m || c.count(v)) {
					pduel->write_buffer8(MSG_RETRY);
					return FALSE;
				}
				c.insert(v);
				int32_t o1, o2;
				field::get_sum_params(core.select_cards[v]->sum_param, o1, o2);
				int32_t ms = (o2 && o2 < o1) ? o2 : o1;
				sum += ms;
				mx += std::max(o1, o2);
				if(ms < mn)
					mn = ms;
			}
			if(mx < acc || sum - mn >= acc) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			return TRUE;
		}
	}
	return TRUE;
}
int32_t field::sort_card(int16_t step, uint8_t playerid) {
	if(step == 0) {
		returns.bvalue[0] = 0;
		if((playerid == 1) && (core.duel_options & DUEL_SIMPLE_AI)) {
			returns.bvalue[0] = 0xff;
			return TRUE;
		}
		if(core.select_cards.empty())
			return TRUE;
		if (core.select_cards.size() > UINT8_MAX)
			core.select_cards.resize(UINT8_MAX);
		pduel->write_buffer8(MSG_SORT_CARD);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8((uint8_t)core.select_cards.size());
		for(auto& pcard : core.select_cards) {
			pduel->write_buffer32(pcard->data.code);
			pduel->write_buffer8(pcard->current.controler);
			pduel->write_buffer8(pcard->current.location);
			pduel->write_buffer8(pcard->current.sequence);
		}
		return FALSE;
	} else {
		if(returns.bvalue[0] == 0xff)
			return TRUE;
		std::set<uint8_t> c;
		int32_t m = (int32_t)core.select_cards.size();
		for(int32_t i = 0; i < m; ++i) {
			uint8_t v = returns.bvalue[i];
			if(v >= m || c.count(v)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			c.insert(v);
		}
		return TRUE;
	}
	return TRUE;
}
int32_t field::announce_race(int16_t step, uint8_t playerid, int32_t count, int32_t available) {
	if(step == 0) {
		int32_t scount = 0;
		for(uint32_t ft = 0x1; ft < (0x1U << RACES_COUNT); ft <<= 1) {
			if(ft & available)
				++scount;
		}
		if(scount <= count) {
			count = scount;
			core.units.begin()->arg1 = (count << 16) + playerid;
		}
		pduel->write_buffer8(MSG_ANNOUNCE_RACE);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(count);
		pduel->write_buffer32(available);
		return FALSE;
	} else {
		int32_t rc = returns.ivalue[0];
		int32_t sel = 0;
		for(uint32_t ft = 0x1; ft < (0x1U << RACES_COUNT); ft <<= 1) {
			if(!(ft & rc))
				continue;
			if(!(ft & available)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			++sel;
		}
		if(sel != count) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		pduel->write_buffer8(MSG_HINT);
		pduel->write_buffer8(HINT_RACE);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(returns.ivalue[0]);
		return TRUE;
	}
	return TRUE;
}
int32_t field::announce_attribute(int16_t step, uint8_t playerid, int32_t count, int32_t available) {
	if(step == 0) {
		int32_t scount = 0;
		for(int32_t ft = 0x1; ft != 0x80; ft <<= 1) {
			if(ft & available)
				++scount;
		}
		if(scount <= count) {
			count = scount;
			core.units.begin()->arg1 = (count << 16) + playerid;
		}
		pduel->write_buffer8(MSG_ANNOUNCE_ATTRIB);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8(count);
		pduel->write_buffer32(available);
		return FALSE;
	} else {
		int32_t rc = returns.ivalue[0];
		int32_t sel = 0;
		for(int32_t ft = 0x1; ft != 0x80; ft <<= 1) {
			if(!(ft & rc)) continue;
			if(!(ft & available)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
			++sel;
		}
		if(sel != count) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		pduel->write_buffer8(MSG_HINT);
		pduel->write_buffer8(HINT_ATTRIB);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(returns.ivalue[0]);
		return TRUE;
	}
	return TRUE;
}
static int32_t is_declarable(card_data const& cd, const std::vector<uint32_t>& opcode) {
	std::stack<int32_t> stack;
	for(auto& it : opcode) {
		switch(it) {
		case OPCODE_ADD: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(lhs + rhs);
			}
			break;
		}
		case OPCODE_SUB: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(lhs - rhs);
			}
			break;
		}
		case OPCODE_MUL: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(lhs * rhs);
			}
			break;
		}
		case OPCODE_DIV: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(lhs / rhs);
			}
			break;
		}
		case OPCODE_AND: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(static_cast<int32_t>(lhs && rhs));
			}
			break;
		}
		case OPCODE_OR: {
			if(stack.size() >= 2) {
				int32_t rhs = stack.top();
				stack.pop();
				int32_t lhs = stack.top();
				stack.pop();
				stack.push(static_cast<int32_t>(lhs || rhs));
			}
			break;
		}
		case OPCODE_NEG: {
			if(stack.size() >= 1) {
				int32_t val = stack.top();
				stack.pop();
				stack.push(-val);
			}
			break;
		}
		case OPCODE_NOT: {
			if(stack.size() >= 1) {
				int32_t val = stack.top();
				stack.pop();
				stack.push(static_cast<int32_t>(!val));
			}
			break;
		}
		case OPCODE_ISCODE: {
			if(stack.size() >= 1) {
				int32_t code = stack.top();
				stack.pop();
				stack.push(cd.code == code);
			}
			break;
		}
		case OPCODE_ISSETCARD: {
			if(stack.size() >= 1) {
				int32_t set_code = stack.top();
				stack.pop();
				bool res = cd.is_setcode(set_code);
				stack.push(res);
			}
			break;
		}
		case OPCODE_ISTYPE: {
			if(stack.size() >= 1) {
				int32_t val = stack.top();
				stack.pop();
				stack.push(cd.type & val);
			}
			break;
		}
		case OPCODE_ISRACE: {
			if(stack.size() >= 1) {
				int32_t race = stack.top();
				stack.pop();
				stack.push(cd.race & race);
			}
			break;
		}
		case OPCODE_ISATTRIBUTE: {
			if(stack.size() >= 1) {
				int32_t attribute = stack.top();
				stack.pop();
				stack.push(cd.attribute & attribute);
			}
			break;
		}
		default: {
			stack.push(it);
			break;
		}
		}
	}
	if(stack.size() != 1 || stack.top() == 0)
		return FALSE;
	return cd.code == CARD_MARINE_DOLPHIN || cd.code == CARD_TWINKLE_MOSS
		|| (!cd.alias && (cd.type & (TYPE_MONSTER | TYPE_TOKEN)) != (TYPE_MONSTER | TYPE_TOKEN));
}
int32_t field::announce_card(int16_t step, uint8_t playerid) {
	if(step == 0) {
		pduel->write_buffer8(MSG_ANNOUNCE_CARD);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8((uint8_t)core.select_options.size());
		for(auto& option : core.select_options)
			pduel->write_buffer32(option);
		return FALSE;
	} else {
		int32_t code = returns.ivalue[0];
		card_data data;
		::read_card(code, &data);
		if(!data.code) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		} else {
			if(!is_declarable(data, core.select_options)) {
				pduel->write_buffer8(MSG_RETRY);
				return FALSE;
			}
		}
		pduel->write_buffer8(MSG_HINT);
		pduel->write_buffer8(HINT_CODE);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(code);
		return TRUE;
	}
	return TRUE;
}
int32_t field::announce_number(int16_t step, uint8_t playerid) {
	if(step == 0) {
		if (core.select_options.size() > UINT8_MAX)
			core.select_options.resize(UINT8_MAX);
		pduel->write_buffer8(MSG_ANNOUNCE_NUMBER);
		pduel->write_buffer8(playerid);
		pduel->write_buffer8((uint8_t)core.select_options.size());
		for(auto& option : core.select_options)
			pduel->write_buffer32(option);
		return FALSE;
	} else {
		int32_t ret = returns.ivalue[0];
		if(ret < 0 || ret >= (int32_t)core.select_options.size()) {
			pduel->write_buffer8(MSG_RETRY);
			return FALSE;
		}
		pduel->write_buffer8(MSG_HINT);
		pduel->write_buffer8(HINT_NUMBER);
		pduel->write_buffer8(playerid);
		pduel->write_buffer32(core.select_options[returns.ivalue[0]]);
		return TRUE;
	}
}
