//===---------- ClangFCO.cpp -- Floating-point Code Optimization ----------===//
//
//  Usage:
//  clang-fco <file1> <file2> ... --
//
//===----------------------------------------------------------------------===//

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/Lexer.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Signals.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;


namespace {
class ClangFCOCallback : public MatchFinder::MatchCallback {
public:
  ClangFCOCallback(std::map<std::string, Replacements> *Replace)
      : Replace(Replace) {}

  void run(const MatchFinder::MatchResult &Result) override {

    const BinaryOperator *BinOp = Result.Nodes.getNodeAs<BinaryOperator>("AssignOp");
    const Expr *LeftHS = BinOp->getLHS();
    const Expr *RightHS = BinOp->getRHS();

    //BinOp->dump();

    llvm::outs() << BinOp->getOperatorLoc().printToString(*(Result.SourceManager)) << ": ";

    llvm::outs() << "match " << BinOp->isMultiplicativeOp() << " " <<
      LeftHS->getType().getAsString() << " " <<
      RightHS->getType().getAsString() << " " <<
      LeftHS->getType().getTypePtr()->isRealFloatingType() << " " << '\n';

    Replacement Rep(*(Result.SourceManager), BinOp->getLocStart(), 0, "/* test */");

    std::map<std::string, Replacements>::iterator it;
    it = Replace->find(Result.SourceManager->getFilename(BinOp->getExprLoc()));
    if (it != Replace->end())
      {
	llvm::outs() << "bingo: " << it->second.size() << '\n';
	Replacements rr(Rep);
	it->second = rr.merge(it->second);
	llvm::outs() << "bingo: " << it->second.size() << '\n';
      }
    else
      {
	Replace->insert ( std::pair<std::string, Replacements>(Result.SourceManager->getFilename(BinOp->getExprLoc()) , Rep) );
      }

    llvm::outs() << Result.SourceManager->getFilename(BinOp->getExprLoc()) << '\n';

    llvm::outs() << Rep.isApplicable() << " " << Rep.toString() << '\n';

  }

private:
  std::map<std::string, Replacements> *Replace;
};
} // end anonymous namespace

// Set up the command line options
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::OptionCategory ClangFCOCategory("clang-fco options");

int main(int argc, const char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  CommonOptionsParser OptionsParser(argc, argv, ClangFCOCategory);
  RefactoringTool Tool(OptionsParser.getCompilations(),
		       OptionsParser.getSourcePathList());
  ast_matchers::MatchFinder Finder;
  ClangFCOCallback Callback(&Tool.getReplacements());

  // TODO: Put your matchers here.
  // Use Finder.addMatcher(...) to define the patterns in the AST that you
  // want to match against. You are not limited to just one matcher!

  Finder.addMatcher( binaryOperator(hasOperatorName("=")).bind("AssignOp") , &Callback);

  Finder.addMatcher( varDecl(hasInitializer(binaryOperator(hasOperatorName("*")).bind("AssignOp"))).bind("someVarDecl") , &Callback);

  int res = Tool.run/*AndSave*/(newFrontendActionFactory(&Finder).get());

  llvm::outs() << Tool.getReplacements().size() << '\n';

  return res;
}
