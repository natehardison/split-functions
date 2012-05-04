/***   PrintFunctionInfo.cpp   ***********************************************
 * This code is licensed under the New BSD license.
 * See LICENSE.txt for details.
 *
 * Author: Nate Hardison
 * 
 * Based largely on the CI Tutorials from:
 *   https://github.com/loarabia/Clang-tutorial/wiki/TutorialOrig
 *
 * Also inspired by the PrintFunctionNames clang plugin example
 *****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#include <iostream>

#include "llvm/Support/Host.h"

#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Parse/Parser.h"
#include "clang/Parse/ParseAST.h"

using namespace clang;

/******************************************************************************
 *
 *****************************************************************************/
class FunctionLocConsumer : public ASTConsumer
{
public:
    FunctionLocConsumer() : ASTConsumer() { }
    virtual ~FunctionLocConsumer() { }

    virtual void Initialize(ASTContext &context) {
        this->sm = &context.getSourceManager();
    }

    virtual bool HandleTopLevelDecl(DeclGroupRef dgr)
    {
        /* We're going to pull out all defined functions that we can find that
         * aren't in the system header files */
        for (DeclGroupRef::iterator it = dgr.begin(); it != dgr.end(); it++)
        {
            if (const FunctionDecl *fd = llvm::dyn_cast<FunctionDecl>(*it))
            {
                if (fd->hasBody(fd))
                {
                    SourceLocation begin = fd->getSourceRange().getBegin();
                    if (!sm->isInSystemHeader(begin))
                    {
                        PrintFunctionInfo(fd);
                    }
                }
            }
        }
        return true;
    }

private:
    /* We print this in a format that Python can easily understand and parse */
    void PrintFunctionInfo(const FunctionDecl *fd)
    {
        SourceRange sr = fd->getSourceRange();
        PresumedLoc begin = sm->getPresumedLoc(sr.getBegin());
        PresumedLoc end = sm->getPresumedLoc(sr.getEnd());

        std::cout << "{'function': '" << fd->getDeclName().getAsString() << "'";
        std::cout << ", 'file': '" << begin.getFilename() << "'";
        std::cout << ", 'begin': [" << begin.getLine() << ", " << begin.getColumn() << "]";
        std::cout << ", 'end': [" << end.getLine() << ", " << end.getColumn() << "]";
        std::cout << "}" << std::endl;
    }

    SourceManager *sm;
};

/******************************************************************************
 *
 *****************************************************************************/
int
main(int argc, const char *argv[])
{
    struct stat sb;

    if (argc != 2)
    {
        std::cout << "Usage: ./PrintFunctionInfo <filename>" << std::endl;
        exit(1);
    }

    if (stat(argv[1], &sb) == -1)
    {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }

    CompilerInstance ci;
    ci.createDiagnostics(0,NULL);

    TargetOptions to;
    to.Triple = llvm::sys::getDefaultTargetTriple();
    TargetInfo *pti = TargetInfo::CreateTargetInfo(ci.getDiagnostics(), to);
    ci.setTarget(pti);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());
    ci.createPreprocessor();
    ci.getPreprocessorOpts().UsePredefines = false;
    FunctionLocConsumer *astConsumer = new FunctionLocConsumer();
    ci.setASTConsumer(astConsumer);

    ci.createASTContext();

	const FileEntry *pFile = ci.getFileManager().getFile(argv[1]);
    ci.getSourceManager().createMainFileID(pFile);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
                                             &ci.getPreprocessor());
    ParseAST(ci.getPreprocessor(), astConsumer, ci.getASTContext());
    ci.getDiagnosticClient().EndSourceFile();

    return 0;
}
