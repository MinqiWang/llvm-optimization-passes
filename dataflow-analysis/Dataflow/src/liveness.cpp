#include "dfa/framework.h"

// @TODO 

namespace {
class Liveness : public dfa::Framework < Value *, 
	                                  dfa::Direction::Backward > // @TODO Change the base class if necessary.
{
protected:
	// Override this print method to print the value of "Value *" and to print
	// "NOT AVAILABLE" for PHI instructions
	virtual void __dumpDomainWithMask(const BitVector & mask, const Instruction & inst) const override final
	{
		LLVM_ASSERT_W_MSG(_domain.size() == mask.size(), 
			_domain.size() << " != " << mask.size());
		
		if (isa <PHINode> (&inst))
		{
			outs() << "NOT AVAILABLE";
		}
		else{
			outs() << "{";

			unsigned mask_idx = 0;
			for (auto & element : _domain)
			{
				if (!mask[mask_idx++])
				{
					continue;
				}
				element->printAsOperand(outs(), false); outs() << ", ";
			}
			outs() << "}";
		}
	}

	virtual void _initializeDomain(const Function & func) override final
	{
		for (auto & bb : func)
		{
			for (auto & inst : bb)
			{
				for (auto opIter = inst.op_begin(); opIter != inst.op_end(); ++opIter)
				{
					Value * val = *opIter;
					if (isa < Instruction > (val) || isa <Argument> (val))
					{
						// Valid variable that we are interested in, add it to the domain set
						_domain.insert(val);
					}
				}
			}
		}
	}
	virtual BitVector __getBoundaryCondition(const Function & func, 
											 const BasicBlock & bb) const override final
	{
		BitVector * bc = new BitVector(_domain.size(), false); // Empty set
		return *bc;
	}
	virtual void _initializeInstBVMap(const Function & func) override final
	{
		for (auto & bb : func)
		{
			for (auto & inst : bb)
			{
				_inst_bv_map.insert(std::make_pair(&inst, 
										   		BitVector(_domain.size(), false))); // All empty sets
			}
		}
	}
	virtual BitVector __meetOp(const BasicBlock & bb) override final
	{
		BitVector * result = new BitVector(_domain.size(), false);
		for (const BasicBlock * Succ : successors(&bb))
		{
			auto iter = Succ->begin();
			BitVector bvExcluded = BitVector(_domain.size(), true);
			while (isa < PHINode > (iter)) // Skip all the phi instructions because we don't
										   // have results stored for them
										   // However, we still have to exclude the definitions
										   // in the phi instructions from the INPUT live set
										   // coming from this successor.
			{
				int phiIdx = lookup(&(*iter));
				bvExcluded[phiIdx] = false;
				++iter;
			}
			if (iter != Succ->end()) // Actually we shouldn't reach the end
			{
				BitVector & bv = _inst_bv_map.at(&(*iter));
				bvExcluded &= bv;
				*result |= bvExcluded; // Meet op is a big "or"
			}
		}
		return *result;
	}
	virtual bool __instTransferFunc(const Instruction & inst,
									const BitVector & ibv, BitVector & obv) override final
	{
		bool result = false;
		BitVector newbv = ibv;

		if (const PHINode * PhiInst = dyn_cast <PHINode> (&inst))
		{
			for (unsigned node_idx = 0; node_idx < PhiInst->getNumIncomingValues(); ++node_idx)
			{
				// For each operand, we have to record that it is live at the OUTPUT
				// of its incoming block. However, since we only store the INPUT of each
				// instruction (in _inst_bv_map) but not the OUTPUT, here we record 
				// the operand is live at the INPUT of the last instruction only if that
				// instruction does not kill (i.e. define) this operand.
				Value * val = PhiInst->getIncomingValue(node_idx);
				BasicBlock * bb = PhiInst->getIncomingBlock(node_idx);
				auto lastInstPtr = (--(bb->end()));
				if (val != (Value *)&(*lastInstPtr))
				{
					BitVector & bv1 = _inst_bv_map.at(&(*lastInstPtr));
					int bv1_idx = lookup(val);
					if (bv1_idx != -1)
					{
						if (bv1[bv1_idx] != true)
						{
							bv1[bv1_idx] = true;
							result = true;
						}
					}
				}
			}
		}
		else
		{
			// Remove the definition from the live set
			int defIdx = lookup(&inst);
			if (defIdx != -1)
			{
				newbv[defIdx] = false;
			}
			// Add uses to the live set
			for (auto opIter = inst.op_begin(); opIter != inst.op_end(); ++opIter)
			{
				Value * val = *opIter;
				int valIdx = lookup(val);
				if (valIdx != -1)
				{
					newbv[valIdx] = true;
				}
			}
			if (obv != newbv)
			{
				BitVector oldOBV = BitVector(obv);
				obv |= newbv;
				if (obv != oldOBV)
				{
					result = true;
				}
			}
		}

		return result; // Should be result
	}
	int lookup(const Value * val)
	{
		unsigned result = 0;
		for (auto & element : _domain)
		{
			if (val == element)
			{
				return result;
			}
			result++;
		}
		return -1;
	}

public:
	static char ID;

	Liveness() : dfa::Framework < Value *,
								dfa::Direction::Backward >(ID) {}
};

char Liveness::ID = 1; 
RegisterPass < Liveness > Y ("liveness", "Liveness");
} // namespace anonymous