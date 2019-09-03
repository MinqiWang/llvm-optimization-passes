#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/ValueTracking.h"

#include <vector>
#include <algorithm>

using namespace llvm;

namespace {
class LICM : public LoopPass
{
private:
	std::vector <Instruction *> invariants;

public:
	static char ID;

	LICM() : LoopPass(ID)
	{}

	~LICM()
	{}

	virtual void getAnalysisUsage(AnalysisUsage & AU) const
	{
		AU.addRequired<DominatorTreeWrapperPass>();
		AU.setPreservesCFG();
	}

	virtual bool testCMConditions(const Instruction * I)
	{
		return isSafeToSpeculativelyExecute(I) && !I->mayReadFromMemory() && !isa < LandingPadInst > (I);
	}

	virtual bool testDominateExitingBlocks(const Instruction * I, SmallVectorImpl<BasicBlock *> & exitingBlocks, DominatorTree & DT)
	{
		bool result = true;
		for (BasicBlock * exitBlock : exitingBlocks)
		{
			if (!DT.dominates(I, exitBlock))
			{
				result = false;
				break;
			}
		}
		return result;
	}

	virtual bool findInvariants(Loop *L)
	{
		bool if_found_invariants = false;
		for (auto blockIter = L->block_begin(); blockIter != L->block_end(); ++blockIter)
		{
			BasicBlock & bb = **blockIter;
			for (auto instIter = bb.begin(); instIter != bb.end(); ++instIter)
			{
				Instruction & inst = *instIter;
				// The instruction is a loop invariant if all of its operands are loop invariants
				if (std::find(invariants.begin(), invariants.end(), &inst) == invariants.end())
				{
					// We don't want to check the instruction again if it has already been verified to an invariant
					bool verified_invariant = true;
					for (auto opIter = inst.op_begin(); opIter != inst.op_end(); ++opIter)
					{
						Value * operand = *opIter;
						if (Instruction * instOperand = dyn_cast<Instruction>(operand))
						{
							if (std::find(invariants.begin(), invariants.end(), instOperand) == invariants.end())
							{
								// Check if this operand is defined outside the loop, if yes, then this is an invariant
								if (L->contains(instOperand))
								{
									// We can't say whether or not this operand is an invariant yet if it is neither defined outside the loop nor already a verified invariant
									verified_invariant = false;
								}
								else
								{	
									invariants.push_back(instOperand);
								}
							}
						}
					}
					if (verified_invariant)
					{
						invariants.push_back(&inst);
						if_found_invariants = true;
					}
				}
			}
		}
		return if_found_invariants;
	}

	virtual bool runOnLoop(Loop *L, LPPassManager & LPM)
	{
		bool changed = false;
		SmallVector<BasicBlock *, 8> exitingBlocks;
		L->getExitingBlocks(exitingBlocks);
		DominatorTree & DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();

		BasicBlock * preHeader = L->getLoopPreheader();
		if (preHeader != NULL)
		{
			// Find all loop invariants
			bool found_more_invariants = true;
			while (found_more_invariants)
			{
				found_more_invariants = findInvariants(L);
			}

			// Move all of the invariants that dominates all the exits to the preheader
			auto InsertPt = preHeader->getTerminator();
			for (Instruction * invariant : invariants)
			{
				if (testDominateExitingBlocks(invariant, exitingBlocks, DT) && testCMConditions(invariant))
				{
					// Move the instruction to preHeader
					invariant->moveBefore(InsertPt);
					changed = true;
				}
			}

		}
		return changed;
	}
};

char LICM::ID = 0;

RegisterPass < LICM > X ("loop-invariant-code-motion", "CSCD70: Loop Invariant Code Motion");
}