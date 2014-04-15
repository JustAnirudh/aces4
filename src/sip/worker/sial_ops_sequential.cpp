/*
 * sial_ops_sequential.cpp
 *
 *  Created on: Apr 8, 2014
 *      Author: basbas
 */

#include "sial_ops_sequential.h"
#include "interpreter.h"

namespace sip {

//#ifndef HAVE_MPI

SialOpsSequential::SialOpsSequential(DataManager & data_manager,
		PersistentArrayManager<Block, Interpreter> * persistent_array_manager):
sip_tables_(SipTables::instance()),
data_manager_(data_manager),
block_manager_(data_manager.block_manager_),
persistent_array_manager_(persistent_array_manager) {
}

SialOpsSequential::~SialOpsSequential() {

}

/** the sequential version is a no-op.
 * TODO consider adding data race detection to the sequential version
 */
void SialOpsSequential::sip_barrier() {
	/*is empty for the sequential, serverless version.  May want to add data race detection.
	 */
}

/** Currently this is a no-op in both parallel and sequential implementations
 *   Map, and blocks are created lazily when needed
 */
void SialOpsSequential::create_distributed(int array_id) {
}

void SialOpsSequential::delete_distributed(int array_id) {
	/* Removes all the blocks associated with this array from the block map.
	 * In this implementation, removing it from the map will cause its destructor to
	 * be called, which will delete the data. Because of this, we must be very
	 * careful to remove blocks that should not be delete from the block_map_.
	 */
	//work delegated to block_manager_
	block_manager_.delete_per_array_map_and_blocks(array_id);
}

void SialOpsSequential::get(BlockId& block_id) {
	get_block_for_reading(block_id);
}

/** A put appears in a SIAL program as
 * put Target(i,j,k,l) = Source(i,j,k,l)
 *
 * In sequential aces4, this is just a copy.
 *
 * @param target
 * @param source_ptr
 */
void SialOpsSequential::put_replace(BlockId& target_id,
		const Block::BlockPtr source_block) {
	Block::BlockPtr target_block = get_block_for_writing(target_id, false);
	target_block->copy_data_(source_block);
}

void SialOpsSequential::put_accumulate(BlockId& target_id,
		const Block::BlockPtr source_block) {
	// Accumulate locally
	Block::BlockPtr target_block = block_manager_.get_block_for_accumulate(target_id, false);
	target_block->accumulate_data(source_block);
}

void SialOpsSequential::destroy_served(int array_id) {
	delete_distributed(array_id);
}

void SialOpsSequential::request(BlockId& block_id) {
	get(block_id);
}
void SialOpsSequential::prequest(BlockId&, BlockId&) {
	fail("PREQUEST not supported !");
}
void SialOpsSequential::prepare(BlockId& lhs_id, Block::BlockPtr source_ptr) {
	put_replace(lhs_id, source_ptr);
}
void SialOpsSequential::prepare_accumulate(BlockId& lhs_id,
		Block::BlockPtr source_ptr) {
	put_accumulate(lhs_id, source_ptr);
}




void SialOpsSequential::collective_sum(int source_array_slot,
               int dest_array_slot) {
       double val_source = data_manager_.scalar_value(source_array_slot);
       double val_dest = data_manager_.scalar_value(dest_array_slot);
       double summed = 0.0;
       summed = val_source + val_dest;
       data_manager_.set_scalar_value(dest_array_slot, summed);
}


void SialOpsSequential::set_persistent(Interpreter* interpreter,
		int array_id, int string_slot) {
	persistent_array_manager_->set_persistent(interpreter, array_id, string_slot);
}

void SialOpsSequential::restore_persistent(Interpreter* interpreter, int array_id, int string_slot) {
	persistent_array_manager_->restore_persistent(interpreter, array_id, string_slot);
}

void SialOpsSequential::end_program() {
}

/** wrapper around these methods in the block_manager.  Checks for data
 * races due to missing barrier and implements the wait for blocks of
 * distributed and served arrays.
 *
 * @param id
 * @return
 */
Block::BlockPtr SialOpsSequential::get_block_for_reading(const BlockId& id) {
	return block_manager_.get_block_for_reading(id);
}

Block::BlockPtr SialOpsSequential::get_block_for_writing(const BlockId& id,
		bool is_scope_extent) {
	return block_manager_.get_block_for_writing(id, is_scope_extent);
}

Block::BlockPtr SialOpsSequential::get_block_for_updating(const BlockId& id) {
	return block_manager_.get_block_for_updating(id);
}

//#else

//#endif
}
/* namespace sip */
