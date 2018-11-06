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
  // TODO done 1106 1814
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

  if( tok_vec.size() == 0 ){
    return CmdExec::errorOption( CMD_OPT_MISSING, "" );
  }


  // tok_vec.size() >= 1;
  int size_per_arr = 0;
  int times = 0;
  int flag = 0;

  if( myStr2Int( tok_vec[0], times ) ){ // "mtn x -a y" or "mtn x";
    if( tok_vec.size() == 1 ){
      mtest.newObjs( times );
      return CMD_EXEC_DONE;
    }else{ // tok_vec.size() >= 2; "mtn x ...";
      // check if "-Array" is specified.

      flag = myStrNCmp( "-Array", tok_vec[1], 2 );
      if( flag != 0 ){
        // extra option
        return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[2] );
      }else{
        // tok_vec[2] matches "-Array", check size_per_arr;
        // "mtn x -a"...

        if( tok_vec.size() == 2 ){
          // "mtn x -a"
          return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[1] );
        }else{ // tok_vec.size() >= 3; "mtn x -a str" where str != "";
          if( !myStr2Int( tok_vec[2], size_per_arr )){
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2] );
          }else{
            if( tok_vec.size() >= 4 ){
              return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[3] );
            }else{
              mtest.newArrs( times, size_per_arr );
              return CMD_EXEC_DONE;
            }
          } // end of if (tok_vec[2] is a number );
        } // end of ( tok_vec.size() >= 3 );
      } // end of ( flag == 0 ), "mtn -x -a ..."
    } // end of (tok_vec.size() >= 2 );
  }else{ // end of "mtn # ..."; shall be "mtn tok ..." where 'tok' is not number.
    
    flag = myStrNCmp( "-Array", tok_vec[0], 2 );
    if( flag != 0 ){
      return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[0] );
    }else{ // "mtn -a ..."
      if( tok_vec.size() == 1 ){
        return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[0] );
      }else if( tok_vec.size () == 2 ){
        if( ! myStr2Int( tok_vec.back(), arr_cnt ) ){
          // "mtn -a not_a_num"
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec.back() );
        }else{
          // "mtn -a x"
          return CmdExec::errorOption( CMD_OPT_MISSING, "" );
        }
      }else{ // tok_vec.size() >= 3
        if( ! myStr2Int( tok_vec[1], size_per_arr ) ){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[1] );
        }
        if( !myStr2Int( tok_vec[3], times )){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[3] );
        }
        // "const string& option" shall be "mtn -a x y ..." now
        if( tok_vec.size() >= 4 ){
          return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[3] );
        }else{
          mtest.newArrs( times, size_per_arr );
        }
      }// end of (tok_vec.size() >= 4 )
    }// end of ( option == "mtn -a ..." );
  } // end of if( "mtn x ..." ) else ( "mtn -arr" ) {}

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

  string _error_msg_Human[] = {
    "okay",
    "arr_size_is_0",
    "idx_size_is_0",
    "no_method_defined",
    "no_index_idx_specified",
    "no_rndom_times_specified",
    "invalid_number",
    "just_an_error"
  };

  method = dummy;
  bool array = false;
  bool something_wrong = false;
  size_t  count = 0;       // for 'x' in "-Index x" or "-Random x"
  int  flag  = 0;          // used in util.h::myStrNCmp
  size_t arr_tok_idx = 0;  // index for token "-Array" in tok_vec_org;
  size_t idx_tok_idx = 0;  // index for token "-Index" in tok_vec_org;
  size_t rn__tok_idx = 0;  // index for token "-Random" in tok_vec_org;
  size_t cnt_tok_idx = 0;  // index for token "objId", "numRandId" in tok_vec_org;
  vector<_error_msg> error_msg_arr;

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
  for( size_t i = 0; i < tok_vec.size(); ++i ){
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
          error_msg_arr[i+1] = invalid__num;
          tok_vec[i+1] = "";
          break;
        }
      } // if( i == tok_vec.size() -1 ) else {}
    }// if "-Index"
  }

  // first step part II, we try "-Random x" here.
  if( method == dummy ){
    for( size_t i = 0; i < tok_vec.size(); ++i ){
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
            error_msg_arr[i+1] = invalid__num;
            tok_vec[i+1] = "";
            break;
          }
        } // if( i == tok_vec.size() -1 ) else {}
      }// if "-Index"
    }
  }// if ( method == dummy );


  // second, use loop to get "-Array" flag
  for( size_t i = 0; i < tok_vec.size(); ++i ){
    flag = myStrNCmp( "-Array", tok_vec[i], 2 );
    if( flag == 0 ){
      array = true;
      arr_tok_idx = i;
      tok_vec[i] = "";
      break;
    }
  }

  if( method != index && method != rndom ){
    if( error_msg_arr.back() == okay )
      error_msg_arr.back() = no_method;
    something_wrong = true;
  }

  if( method == index ){
    if( array ){
      if( mtest.getArrListSize() == 0 || mtest.getArrListSize()-1 < count ){
        cerr << "Size of array list (" << mtest.getArrListSize()
          << ") is <= " << count << "!!" << endl;
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = invalid__num;
      }
    }else{
      if( mtest.getObjListSize() == 0 || mtest.getObjListSize()-1 < count ){
        cerr << "Size of object list (" << mtest.getObjListSize()
          << ") is <= " << count << "!!" << endl;
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = invalid__num;
      }
    }
  }

  if( method == rndom ){
    if( array ){
      if( mtest.getArrListSize() == 0 ){
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = invalid__num;
      }
    }else{
      if( mtest.getObjListSize() == 0 ){
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = invalid__num;
      }
    }
  }


  // third, check if there's error happened.
  for(auto it = tok_vec.begin(); it != tok_vec.end(); ++it){
    if( *it != "" ){
      error_msg_arr[(it - begin )] = just_an_error;
    }
  }
  if( !tok_vec.empty() )
    something_wrong = true;

  if( something_wrong == false ){
    for( auto& it : error_msg_arr ){
      if( it != okay ){
        something_wrong = true;
        break;
      }
    }
  }

  if( something_wrong ){
#ifdef MEM_DEBUG
    for( size_t i = 0; i < tok_vec_org.size(); ++i ){
      cerr << tok_vec_org[i] << '\t';
      if( i%5 == 0 && i != 0 )
        cerr << endl;
    }
    for( size_t i = 0; i < error_msg_arr.size(); ++i ){
      cerr << _error_msg_Human[error_msg_arr[i]] << '\t';
      if( i%5 == 0 && i != 0 )
        cerr << endl;
    }
#endif // MEM_DEBUG
    for( size_t i = 0; i < tok_vec_org.size(); ++i ){
      switch( error_msg_arr[i] ){ // now working....
        case okay:
          continue;
        case arr_size_0:
          cerr << "Size of array list is " << mtest.getArrListSize()
            << "!!" << endl;
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
        case idx_size_0:
          cerr << "Size of object list is " << mtest.getObjListSize()
            << "!!" << endl;
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
        case no_method:
          for( auto& it : tok_vec ){
            if( it != "" )
              return CmdExec::errorOption( CMD_OPT_ILLEGAL, it );
          }
          return CmdExec::errorOption( CMD_OPT_MISSING, "" );
        case no_index_num:
          return CmdExec::errorOption( CMD_OPT_MISSING, 
              tok_vec_org[idx_tok_idx] );
        case no_rndom_num :
          return CmdExec::errorOption( CMD_OPT_MISSING,
              tok_vec_org[rn__tok_idx] );
        case invalid__num:
          return CmdExec::errorOption( CMD_OPT_ILLEGAL,
              tok_vec_org[i] );
        case just_an_error:
          return CmdExec::errorOption( CMD_OPT_ILLEGAL,
              tok_vec_org[i] );
        default:
          assert( 0 && "WTF in MTDeleteCmd::exec(const string& )" );
      }
#ifdef MEM_DEBUG
      assert( 0 && "WTF in MTDeleteCmd::exec(const string& )" );
#endif // MEM_DEBUG
    } // end for loop
  } // end of ( something_wrong == true );

  // finally, do what "const string& option" want us to do.
  if( _method == rndom ){
    if( array ){
      for( size_t i = 0; i < count; ++i ){
        mtest.deleteArr( rnGen(mtest.getArrListSize() ) );
      }
      else {
        for( size_t i = 0; i < count; ++i ){
          mtest.deleteObj( rnGen(mtest.getObjListSize() ) );
        }
      }
    }
  }else if ( _method == index ){



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


