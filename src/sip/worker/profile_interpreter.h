/*
 * profile_interpreter.h
 *
 *  Created on: Oct 1, 2014
 *      Author: njindal
 */

#ifndef PROFILE_INTERPRETER_H_
#define PROFILE_INTERPRETER_H_

#include "sialx_interpreter.h"
#include "profile_timer.h"

namespace sip {

class ProfileTimer;
class SipTables;
class SialPrinter;
class WorkerPersistentArrayManager;

class ProfileInterpreter: public SialxInterpreter {
public:
	ProfileInterpreter(const SipTables& sipTables, ProfileTimer& profile_timer,
			SialxTimer& sialx_timer, SialPrinter* printer,
			WorkerPersistentArrayManager* persistent_array_manager);
	virtual ~ProfileInterpreter();

	virtual void pre_interpret(int pc);
	virtual void post_interpret(int oldpc, int newpc);
	void profile_timer_trace(int pc, opcode_t opcode);



private:
	SialxTimer& sialx_timer_;			//! Reference to sialx timer. Operated on by SialxInterpreter
	ProfileTimer& profile_timer_;		//! This interpreter records time into ProfileTimer
	ProfileTimer::Key last_seen_key_;
	int last_seen_pc_;
	ProfileTimer::Key make_profile_timer_key(opcode_t opcode, const std::list<BlockSelector>& selector_list);
	ProfileTimer::Key make_profile_timer_key(const std::string& opcode_name, const std::list<BlockSelector>& selector_list);

};

} /* namespace sip */

#endif /* PROFILE_INTERPRETER_H_ */