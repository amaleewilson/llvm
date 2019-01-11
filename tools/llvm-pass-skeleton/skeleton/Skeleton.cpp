#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

// https://lists.llvm.org/pipermail/llvm-dev/2011-November/045478.html
// https://www.cs.cornell.edu/~asampson/blog/llvm.html

namespace {
    struct SkeletonPass : public FunctionPass {
        static char ID;
        SkeletonPass() : FunctionPass(ID) {}

        virtual bool runOnFunction(Function &F) {
            if (F.getName() != "f") {
                errs() << "not modifying function '" << F.getName() << "'\n";
                return false;
            }

            // Function *call_g = Function::Create(F.getParent()->getFunction("g")->getFunctionType(), GlobalValue::ExternalLinkage, "g", F.getParent());
            // errs() << *call_g << "\n";

            LLVMContext &Ctx = F.getContext();
            FunctionType *gType = FunctionType::get(Type::getVoidTy(Ctx), false);
            Constant *gFunc = F.getParent()->getOrInsertFunction("g", gType);

            errs() << "Function: " << F.getName() << "\n";
            for (auto &B : F) {
                errs() << "    Basic Block: " << B.getName() << "\n";

                for (auto &I : B) {
                    if (I.isTerminator()) {
                        IRBuilder <> builder(&I);
                        builder.SetInsertPoint(&B, builder.GetInsertPoint());
                        builder.CreateCall(cast <Function> (gFunc), {});

                        // this works with call_g
                        // ArrayRef <Value *> Args;
                        // CallInst *NewCall = CallInst::Create(call_g, Args, "", &I);
                        break;
                    }

                    // errs() << "        Instruction: " << I.getOpcodeName() << " " << I.isTerminator() << " " << I << "\n";

                    // if (auto *op = dyn_cast<BinaryOperator>(&I)) {
                    //   // Insert at the point where the instruction `op` appears.
                    //   IRBuilder<> builder(op);

                    //   // Make a multiply with the same operands as `op`.
                    //   Value *lhs = op->getOperand(0);
                    //   Value *rhs = op->getOperand(1);
                    //   Value *mul = builder.CreateMul(lhs, rhs);

                    //   // Everywhere the old instruction was used as an operand, use our
                    //   // new multiply instruction instead.
                    //   for (auto &U : op->uses()) {
                    //     User *user = U.getUser();  // A User is anything with operands.
                    //     user->setOperand(U.getOperandNo(), mul);
                    //   }

                    //   // We modified the code.
                    //   return true;
                    // }
                }

                for (auto &I : B) {
                    errs() << "        Instruction: " << I.getOpcodeName() << " " << I.isTerminator() << " " << I << "\n";
                }
            }

            return true;
        }

        virtual StringRef getPassName() const {
            return "SkeletonPass";
        }
    };

    struct SICMPass : public ModulePass {
        static char ID;
        SICMPass() : ModulePass(ID) {}

        virtual bool runOnModule(Module &M) {
            errs() << M.getFunction("@f")->getName() << "\n";
            errs() << M.getName() << "!\n";
            for (auto &F : M) {
                for (auto &B : F) {
                    for (auto &I : B) {
                        errs() << I.getOpcodeName() << "\n";
                    }
                }
            }

            return false;
        }

        virtual StringRef getPassName() const {
            return "SICMPass";
        }
    };
}

char SkeletonPass::ID = 0;
char SICMPass::ID = 0;

// Automatically enable the pass.
// http://adriansampson.net/blog/clangpass.html
static void registerSkeletonPass(const PassManagerBuilder &,
                         legacy::PassManagerBase &PM) {
  PM.add(new SkeletonPass());
  // PM.add(new SICMPass());
}
static RegisterStandardPasses
  RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
                 registerSkeletonPass);
