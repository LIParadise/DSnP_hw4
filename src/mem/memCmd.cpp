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
  // TODO done 1104 2136
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
    }else{ // tok_vec.size() >= 3; "mtn x ...";
      // check if "-Array" is specified.

      flag = myStrNCmp( "-Array", tok_vec[2], 2 );
      if( flag != 0 ){
        // extra option
        return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[2] );
      }else{
        // tok_vec[2] matches "-Array", check arr_cnt;
        // "mtn x -a"...

        if( tok_vec.size() == 3 ){
          // "mtn x -a"
          return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[2] );
        }else{ // tok_vec.size() >= 4; "mtn x -a str" where str != "";
          if( !myStr2Int( tok_vec[3], arr_cnt )){
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[3] );
          }else{
            if( tok_vec.size() >= 5 ){
              return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[4] );
            }else{
              mtest.newArrs( arr_cnt, size_per_arr );
              return CMD_EXEC_DONE;
            }
          } // end of if (tok_vec[3] is a number );
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
        if( ! myStr2Int( tok_vec.back(), arr_cnt ) ){
          // "mtn -a not_a_num"
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec.back() );
        }else{
          // "mtn -a x"
          return CmdExec::errorOption( CMD_OPT_MISSING, "" );
        }
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

  enum _method{
    dummy = 0,
    index = 1,
    random = 2,
    error = 3
  } method;
  method = dummy;
  int flag = -1;
  int count = 0;
  size_t arr_tok_idx = 0;  // index for token "-Array" in tok_vec_org;
  size_t idx_tok_idx = 0;  // index for token "-Index" in tok_vec_org;
  size_t rn__tok_idx = 0;  // index for token "-Random" in tok_vec_org;
  size_t cnt_tok_idx = 0;  // index for token "objId", "numRandId" in tok_vec_org;
  bool array = false;

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
    return CmdExec::errorOption( cmd_opt_missing, "" );
  }

  vector<string> tok_vec_org = tok_vec;

  for( size_t i = 1; i < tok_vec.size(); ++i ){
    flag = myStrNCmp( "-Index", tok_vec[i], 2 );
    if( flag == 0 && tok_vec.size() > (i+1) ){
      if( myStr2Int( tok_vec[i+1], count ) ){
        method = index;
        tok_vec.erase(i+1);
        tok_vec.erase(i);
        idx_tok_idx = i;
        cnt_tok_idx = (i+1);
        break;
      }
      tok_vec.erase(i);
      method = error;
      break;
    }
  }

  if( method == dummy ){
    for( size_t i = 1; i < tok_vec.size(); ++i ){
      flag = myStrNCmp( "-Random", tok_vec[i], 2 );
      if( flag == 0 && tok_vec.size() > (i+1) ){
        if( myStr2Int( tok_vec[i+1], count ) ){
          method = random;
          tok_vec.erase(i+1);
          tok_vec.erase(i);
          rn__tok_idx = i;
          cnt_tok_idx = (i+1);
          break;
        }
        tok_vec.erase(i);
        method = error;
        break;
      }
    }
  }

  if( method == index || method == random ){
    _method method_bak = method;
    method = error;
    for( size_t i = 1; i < tok_vec.size(); ++i ){
      flag = myStrNCmp( "-Array", tok_vec[i], 2 );
      if( flag == 0 ){
        array = true;
        method = method_bak;
        tok_vec.erase[i];
        arr_tok_idx = i;
        break;
      }
    }
  }

  if( method == index || method == random ){
    if( tok_vec.size () == 1 ){

      if( method == index ){
        if( array ){
          if( mtest.getArrListSize() > count ){
            // command parsing okay; just delete.
            mtest.deleteArr( count );
          }else{
            cerr << "Size of array list (" << mtest.getArrListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL,tok_vec_org[cnt_tok_idx]);
          }
        }else{ // "-Array" not specified, check _objList;
          if( mtest.getObjListSize() > count ){
            // command parsing okay; just delete.
            mtest.deleteObj( count );
          }else{
            cerr << "Size of object list (" << mtest.getObjListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL,tok_vec_org[cnt_tok_idx]);
          }
        } // if(array) else {}
      } else { // method == random 

        if( array) {
          if( mtest.getArrListSize() == 0 ){
            cerr << "Size of array list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL,tok_vec_org[rn__tok_idx]);
          }else{
            for( size_t i = 0; i < count; ++i ){
              mtest.deleteArr( rnGen( mtest.getArrListSize() ) );
            }
          }
        }else {
          if( mtest.getObjListSize() == 0 ){
            cerr << "Size of object list is 0!!" << endl;
            return CmdExec::errorOption(CMD_OPT_ILLEGAL,tok_vec_org[rn__tok_idx]);
          }else{
            for( size_t i = 0; i < count; ++i ){
              mtest.deleteObj( rnGen( mtest.getObjListSize() ) );
            }
          }
        }
      }
    } else { // tok_vec.size() != 1
      method = error;
    }
  }




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


