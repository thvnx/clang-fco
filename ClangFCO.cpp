//===---- tools/extra/ToolTemplate.cpp - Template for refactoring tool ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements an empty refactoring tool using the clang tooling.
//  The goal is to lower the "barrier to entry" for writing refactoring tools.
//
//  Usage:
//  clang-fco <cmake-output-dir> <file1> <file2> ...
//
//  Where <cmake-output-dir> is a CMake build directory in which a file named
//  compile_commands.json exists (enable -DCMAKE_EXPORT_COMPILE_COMMANDS in
//  CMake to get this output).
//
//  <file1> ... specify the paths of files in the CMake source tree. This path
//  is looked up in the compile command database. If the path of a file is
//  absolute, it needs to point into CMake's source tree. If the path is
//  relative, the current working directory needs to be in the CMake source
//  tree and the file must be in a subdirectory of the current working
//  directory. "./" prefixes in the relative files will be automatically
//  removed, but the rest of a relative path must be a suffix of a path in
//  the compile command line database.
//
//  For example, to use clang-fco on all files in a subtree of the
//  source tree, use:
//
//    /path/in/subtree $ find . -name '*.cpp'|
//        xargs clang-fco /path/to/build
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
      LeftHS->getType().getTypePtr()->isRealFloatingType() << " " <<
      '\n';

    Replacement Rep(*(Result.SourceManager), BinOp->getLocStart(), 0,
		    "// assign_op\n");
    //Replace->add(Rep);
    Replace->insert ( std::pair<std::string,Replacement>("toto",Rep) );
    llvm::outs() << Rep.isApplicable() << " " << Rep.toString() << '\n';

    // TODO: This routine will get called for each thing that the matchers
    // find.
    // At this point, you can examine the match, and do whatever you want,
    // including replacing the matched text with other text
    //(void)Replace; // This to prevent an "unused member variable" warning;
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

  return Tool.run(newFrontendActionFactory(&Finder).get());
}
