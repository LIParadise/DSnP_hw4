/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
 ****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

  bool
initMemCmd()
{
  if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
        cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
        cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
        cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
       )) {
    cerr << "Registering \"mem\" commands fails... exiting" << endl;
    return false;
  }
  return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
  CmdExecStatus
MTResetCmd::exec(const string& option)
{
  // check option
  string token;
  if (!CmdExec::lexSingleOption(option, token))
    return CMD_EXEC_ERROR;
  if (token.size()) {
    int b;
    if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
      cerr << "Illegal block size (" << token << ")!!" << endl;
      return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
    }
#ifdef MEM_MGR_H
    mtest.reset(toSizeT(b));
#else
    mtest.reset();
#endif // MEM_MGR_H
  }
  else
    mtest.reset();
  return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
  os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
  cout << setw(15) << left << "MTReset: " 
    << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
  CmdExecStatus
MTNewCmd::exec(const string& option)
{
  // TODO
  vector<string> tok_vec;
  string temp;
  size_t pos = 0;
  do{
    pos = myStrGetTok( option, temp, pos, ' ' );
    if( ! temp.empty () ){
      tok_vec.push_back( temp );
    }
  }while ( pos != string :: npos );
  // tok_vec now stores tokenized "const string& option"

  if( tok_vec.size() == 1 ){
    return CmdExec::errorOption( CMD_OPT_MISSING, "" );
  }


  // tok_vec.size() >= 2;
  int size_per_arr = 0;
  int arr_cnt = 0;
  int flag = 0;

  if( myStr2Int( tok_vec[1], size_per_arr ) ){ // "mtn x -a y" or "mtn x";
    if( tok_vec.size() == 2 ){
      mtest.newObjs( size_per_arr );
      return CMD_EXEC_DONE;
    }else{ // tok_vec.size() >= 3;
      // check if "-Array" is specified.

      flag = myStrNCmp( "-Array", tok_vec[2], 2 );
      if( flag != 0 ){
        // extra option
        return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[2] );
      }else{
        // check arr_cnt;
        // tok_vec[2] matches "-Array";
        // "mtn x -a"...

        if( tok_vec.size() == 3 ){
          return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[2] );
        }else{ // tok_vec.size() >= 4;
          if( !myStr2Int( tok_vec[3], arr_cnt )){
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[3] );
          }else{
            if( tok_vec.size() >= 5 ){
              return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[4] );
            }else{
              mtest.newArrs( arr_cnt, size_per_arr );
              return CMD_EXEC_DONE;
            }
          } // end of ( tok_vec[3] is a number );
        } // end of ( tok_vec.size() >= 4 );
      } // end of ( flag == 0 ), "mtn -x -a ..."
    } // end of (tok_vec.size() >= 3 );
  }else{ // end of "mtn x ..."; shall be "mtn tok ..."
    
    flag = myStrNCmp( "-Array", tok_vec[1], 2 );
    if( flag != 0 ){
      return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[1] );
    }else{ // "mtn -a ..."
      if( tok_vec.size() == 2 ){
        return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[1] );
      }else if( tok_vec.size () == 3 ){
        return CmdExec::errorOption( CMD_OPT_MISSING, "" );
      }else{ // tok_vec.size() >= 4
        if( ! myStr2Int( tok_vec[2], arr_cnt ) ){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2] );
        }
        if( !myStr2Int( tok_vec[3], size_per_arr )){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[3] );
        }
        // "const string& option" shall be "mtn -a x y ..." now
        if( tok_vec.size() >= 5 ){
          return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[4] );
        }else{
          mtest.newArrs( arr_cnt, size_per_arr );
        }
      }// end of (tok_vec.size() >= 4 )
    }// end of ( option == "mtn -a ..." );
  }



  // Use try-catch to catch the bad_alloc exception
  return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
  os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
  cout << setw(15) << left << "MTNew: " 
    << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
  CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
  // TODO

  return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
  os << "Usage: MTDelete <-Index (size_t objId) | "
    << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
  cout << setw(15) << left << "MTDelete: " 
    << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
  CmdExecStatus
MTPrintCmd::exec(const string& option)
{
  // check option
  if (option.size())
    return CmdExec::errorOption(CMD_OPT_EXTRA, option);
  mtest.print();

  return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
  os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
  cout << setw(15) << left << "MTPrint: " 
    << "(memory test) print memory manager info" << endl;
}


