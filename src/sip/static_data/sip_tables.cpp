/*! SipTables.cpp
 * 
 *
 *  Created on: Jun 6, 2013
 *      Author: Beverly Sanders
 */

#include "sip_tables.h"
#include <assert.h>
#include <cstddef>
#include "array_constants.h"
#include "interpreter.h"
#include "setup_reader.h"
#include <vector>

namespace sip {



SipTables::SipTables(setup::SetupReader& setup_reader, setup::InputStream& input_file):
	setup_reader_(setup_reader),
	siox_reader_(*this, input_file, setup_reader),
	sialx_lines_(-1){
//	check(SipTables::instance_ == NULL, "attempting to write to non-null SipTables::instance_");
	SipTables::instance_ = this;
}

SipTables::~SipTables() {
	SipTables::instance_ = NULL;
}

SipTables* SipTables::instance_;
SipTables& SipTables::instance(){
	return *instance_;
}

int SipTables::max_timer_slots(){
	if (sialx_lines_ > 0)
		return sialx_lines_;

	int max_line = 0;
	int size = op_table_.size();
	for (int i=0; i<size; i++){
		int line = op_table_.line_number(i);
		if (line > max_line)
			max_line = line;
	}
	sialx_lines_ = max_line;
	return max_line;
}

std::vector<std::string> SipTables::line_num_to_name(){
	std::vector<std::string> lno2name(max_timer_slots()+1, "");
	int size = op_table_.size();
	for (int i=0; i<size; i++){
		int line = op_table_.line_number(i);
		opcode_t opcode = intToOpcode(op_table_.opcode(i));
		if (printableOpcode(opcode))
			lno2name[line] = opcodeToName(opcode);
	}
	return lno2name;
}


std::string SipTables::array_name(int array_table_slot) {
	return array_table_.array_name(array_table_slot);
}
std::string SipTables::scalar_name(int array_table_slot) { //the same as array_table_name
	assert(is_scalar(array_table_slot));
	return array_table_.array_name(array_table_slot);
}
int SipTables::array_rank(int array_table_slot) {
	return array_table_.rank(array_table_slot);
}

int SipTables::array_rank(const sip::BlockId& id){
	return array_table_.rank(id.array_id());
}
bool SipTables::is_scalar(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_scalar_attr(attr);
}
bool SipTables::is_auto_allocate(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_auto_allocate_attr(attr);
}

bool SipTables::is_scope_extent(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_scope_extent_attr(attr);
}

bool SipTables::is_contiguous(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_contiguous_attr(attr);
}

bool SipTables::is_predefined(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_predefined_attr(attr);
}

bool SipTables::is_distributed(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_sip_consistent_attr(attr);
}

bool SipTables::is_served(int array_table_slot) {
	int attr = array_table_.array_type(array_table_slot);
	return sip::is_sip_consistent_attr(attr);
}

int SipTables::num_arrays(){
	return array_table_.entries_.size();
}

int SipTables::num_block_in_array(int array_table_slot){
	return array_table_.entries_[array_table_slot].num_blocks_;
}

int SipTables::int_value(int int_table_slot) {
	return int_table_.value(int_table_slot);
}

std::string SipTables::string_literal(int slot) {
	return string_literal_table_.at(slot);
}
std::string SipTables::index_name(int index_table_slot) {
	return index_table_.index_name(index_table_slot);
}

sip::IndexType_t SipTables::index_type(int index_table_slot) {
	return index_table_.index_type(index_table_slot);
}


sip::BlockShape SipTables::shape(const sip::BlockId& block_id) {
	int array_table_slot = block_id.array_id();
	int rank = array_table_.rank(array_table_slot);
	sip::index_selector_t& selectors = array_table_.index_selectors(array_table_slot);
	int seg_sizes[MAX_RANK];
	for (int i = 0; i < rank; ++i) {
		int index_id = selectors[i];
		if  (is_subindex(index_id)){
			int parent_value = sip::Interpreter::global_interpreter->
					data_manager_.index_value(parent_index(index_id));
			seg_sizes[i] = index_table_.subsegment_extent(index_id, parent_value, block_id.index_values(i));
		}
		else {
		seg_sizes[i] = index_table_.segment_extent(selectors[i],
				block_id.index_values(i));
		}
	}
	std::fill(seg_sizes+rank, seg_sizes + MAX_RANK, 1);
	return sip::BlockShape(seg_sizes);
}

int SipTables::block_size(const BlockId& block_id){
	return shape(block_id).num_elems();
}

sip::BlockShape SipTables::contiguous_array_shape(int array_id){
	int rank = array_table_.rank(array_id);
	sip::index_selector_t& selector = array_table_.index_selectors(array_id);
	sip::segment_size_array_t seg_sizes;
	for (int i = 0; i < rank; ++i){
		seg_sizes[i] = index_table_.index_extent(selector[i]);
	}
	std::fill(seg_sizes+rank, seg_sizes + MAX_RANK, 1);
	return sip::BlockShape(seg_sizes);
}

int SipTables::offset_into_contiguous(int selector, int value){
	return index_table_.offset_into_contiguous(selector, value);
}
sip::index_selector_t& SipTables::selectors(int array_id){
	return array_table_.index_selectors(array_id);
}
int SipTables::lower_seg(int index_table_slot) {
	return index_table_.lower_seg(index_table_slot);
}
int SipTables::num_segments(int index_table_slot) {
	return index_table_.num_segments(index_table_slot);
}
int SipTables::num_subsegments(int index_slot, int parent_segment_value){return index_table_.num_subsegments(index_slot, parent_segment_value);}

bool SipTables::is_subindex(int index_table_slot){return index_table_.is_subindex(index_table_slot);}

int SipTables::parent_index(int subindex_slot){return index_table_.parent(subindex_slot);}


std::ostream& operator<<(std::ostream& os, const SipTables& obj) {
	//index table
	os << "Index Table: " << std::endl;
	os << obj.index_table_;
	os << std::endl;
	//array table
	os << "Array Table: " << std::endl;
	os << obj.array_table_;
	os << std::endl;
	//op table
	os << "Op Table " << std::endl;
	os << obj.op_table_;
	os << std::endl;
	//scalar table
	os << "Scalar Table: (" << obj.scalar_table_.size() << " entries)" <<std::endl;
	for (int i = 0; i < obj.scalar_table_.size(); ++i) {
		double val = obj.scalar_table_[i];
		if (val != 0.0){
			os << i << ":" << obj.scalar_table_[i] << std::endl;
		}
	}
	os << std::endl;
	//int table
	os << "Int Table (predefined symbolic constants):" << std::endl;
	os << obj.int_table_;
	os << std::endl;
	// special instructions
	os << "Special Instructions: " << std::endl;
	os << obj.special_instruction_manager_;
	os << std::endl;
	//string literal table
	os << "String Literal Table:" << std::endl;
	for (int i = 0; i < obj.string_literal_table_.size(); ++i) {
		os << "string_literal_table[" << i << "]= " << obj.string_literal_table_[i] << '\n';
	}
	os << std::endl;
	os << std::endl;
	return os;
}

} /* namespace sip */

