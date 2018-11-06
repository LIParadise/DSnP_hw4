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
    dummy = 0;
    index = 1;
    rndom = 2;
  } method;

  enum _error_msg{
    okay = 0;
    arr_size_0    = 1;
    idx_size_0    = 2;
    no_method     = 3;
    no_index_num  = 4;      // x in "-Index x" missing
    no_rndom_num  = 5;      // y in "-Random y" missing
    invalid__num  = 6;
    just_an_error = 7;
  } ;

  method = dummy;
  vector<_error_msg> error_msg_arr;
  bool array = false;
  bool something_wrong = false;
  int  count = 0;          // for 'x' in "-Index x" or "-Random x"
  int  flag  = 0;          // used in util.h::myStrNCmp
  size_t arr_tok_idx = 0;  // index for token "-Array" in tok_vec_org;
  size_t idx_tok_idx = 0;  // index for token "-Index" in tok_vec_org;
  size_t rn__tok_idx = 0;  // index for token "-Random" in tok_vec_org;
  size_t cnt_tok_idx = 0;  // index for token "objId", "numRandId" in tok_vec_org;

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
  error_msg_arr.resize( tok_vec.size() );
  for( auto& it : error_msg_arr ){
    it = okay;
  }

  /* 
     use tok_vec to get required information, including that about error.

     first, use one loop to get required flags and subsidiary "int count",
     and set corresponding positions in tok_vec to "";
     e.g.: "mtd hello_world -ra 3 bump_of_chicken"
     we get "-ra 3", and set tok_vec to '"mtd" "hello_world" "" "" "bump_of_chicken"';

     second, use one loop to get "-Array" flag, similar to the method in first step.

     finally, check if there's error.
     try to find the error in tok_vec accordingly.
     */

  // first step, part I, we try "-Index x" here.
  for( size_t i = 1; i < tok_vec.size(); ++i ){
    flag = myStrNCmp( "-Index", tok_vec[i], 2 );
    if( flag == 0 ){
      tok_vec[i]  = "";
      idx_tok_idx = i;
      if( i == tok_vec.size() - 1 ){
        error_msg_arr[i] = no_index_num;
        break;
      }else{ // "-Index tok"
        if( myStr2Int( tok_vec[i+1], count ) ){ // "-Index x" found
          method = index;
          cnt_tok_idx = i+1;
          tok_vec[i+1] = "";
          break;
        }else{
          // "-Index tok" where "tok" is not a valid number;
          error_msg[i+1] = invalid__num;
          tok_vec[i+1] = "";
          break;
        }
      } // if( i == tok_vec.size() -1 ) else {}
    }// if "-Index"
  }

  // first step part II, we try "-Random x" here.
  if( method == dummy ){
    for( size_t i = 1; i < tok_vec.size(); ++i ){
      flag = myStrNCmp( "-Random", tok_vec[i], 2 );
      if( flag == 0 ){
        tok_vec[i]  = "";
        rn__tok_idx = i;
        if( i == tok_vec.size() - 1 ){
          error_msg_arr[i] = no_rndom_num;
          break;
        }else{ // "-Random tok"
          if( myStr2Int( tok_vec[i+1], count ) ){ // "-Random x" found
            method = rndom;
            cnt_tok_idx = i+1;
            tok_vec[i+1] = "";
            break;
          }else{
            // "-Random tok" where "tok" is not a valid number;
            error_msg[i+1] = invalid__num;
            tok_vec[i+1] = "";
            break;
          }
        } // if( i == tok_vec.size() -1 ) else {}
      }// if "-Index"
    }
  }// if ( method == dummy );

  if( method != index && method != rndom )
    error_msg_arr[0] = no_method;


  // second, use loop to get "-Array" flag
  for( size_t i = 1; i < tok_vec.size(); ++i ){
    flag = myStrNCmp( "-Array", tok_vec[i], 2 );
    if( flag == 0 ){
      array = true;
      arr_tok_idx = i;
      tok_vec[i] = "";
      break;
    }
  }

  // third, check if there's error happened.
  for(auto it = tok_vec.begin(); it != tok_vec.end(); ){
    if( *it == "" )
      tok_vec.erase( it );
    else
      ++it;
  }
  if( tok_vec.size() > 1 )
    something_wrong = true;

  for( auto& it : error_msg_arr ){
    if( it != okay ){
      something_wrong = true;
      break;
    }
  }

  if( something_wrong ){
    for( auto& it : error_msg_arr ){
      switch( it ){
        case arr_s // now working....
      }
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


