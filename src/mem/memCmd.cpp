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
    if( times <= 0 ){
      return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[0] );
    }
    if( tok_vec.size() == 1 ){
      mtest.newObjs( times );
      return CMD_EXEC_DONE;
    }else{ // tok_vec.size() >= 2; "mtn x tok" where tok != "" and 'x' valid.
      // check if "-Array" is specified.

      flag = myStrNCmp( "-Array", tok_vec[1], 2 );
      if( flag != 0 ){
        // extra option
        // eg: "mtn 30 -big_boob"
        return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[1] );
      }else{
        // tok_vec[1] matches "-Array", check size_per_arr;
        // "mtn x -a..."

        if( tok_vec.size() == 2 ){
          // "mtn x -a"
          return CmdExec::errorOption( CMD_OPT_MISSING, tok_vec[1] );
        }else{ // tok_vec.size() >= 3; "mtn x -a str" where str != "";
          if( !myStr2Int( tok_vec[2], size_per_arr )){
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2] );
          }else{
            if( tok_vec.size() >= 4 ){
              return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[3] );
            }else if( size_per_arr <= 0 ){
              return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2]);
            }else{
              try{
                mtest.newArrs( times, size_per_arr );
              }catch( bad_alloc& ba ) {
                return CMD_EXEC_ERROR;
              }
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
        if( ! myStr2Int( tok_vec.back(), size_per_arr ) ){
          // "mtn -a not_a_num"
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec.back() );
        }else{
          // "mtn -a x"
          if( size_per_arr > 0 ){
            return CmdExec::errorOption( CMD_OPT_MISSING, "" );
          }else{
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[1] );
          }
        }
      }else{ // tok_vec.size() >= 3; "mtn -arr tok_a tok_b" where tok_b != "";
        if( ! myStr2Int( tok_vec[1], size_per_arr ) ){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[1] );
        }
        if( size_per_arr <= 0 )
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[1] );
        if( !myStr2Int( tok_vec[2], times )){
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2] );
        }
        if( times < 0 )
          return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec[2] );
        // "const string& option" shall be "mtn -a x y ..." now
        if( tok_vec.size() >= 4 ){
          return CmdExec::errorOption( CMD_OPT_EXTRA, tok_vec[3] );
        }else{
          try{
            mtest.newArrs( times, size_per_arr );
          }catch( bad_alloc& ba ) {
            return CMD_EXEC_ERROR;
          }
        }
      }// end of (tok_vec.size() >= 3 )
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
  // TODO ... done 1106 1949

  enum _method{
    dummy = 0,
    index, 
    rndom,
  } method;

  enum _error_msg{
    okay       = 0,
    no_method     ,
    no_index_num  ,      // x in "-Index x" missing
    no_rndom_num  ,      // y in "-Random y" missing
    index_idx_OoR ,      // x in "-Index x" out of range
    rndom_idx_OoR ,      // y in "-Index y" out of range 
    num_is_0      ,      // delete zero elements, happens when "-Random";
    invalid__num  ,      // 'tok' in "-Index tok" or "-Random tok" invalid
    i_have_no_cash,      // requested thing have size == 0, either obj or arr.
    just_an_error ,
  };

  string _error_msg_Human[] = {
    "okay",
    "no_method_defined",
    "no_index_idx_specified",
    "no_rndom_times_specified",
    "index_#_out_of_range",
    "rndom_#_out_of_range",
    "delete_zero_elements???",
    "invalid_number",
    "i_have_no_money_QQ",
    "just_an_error"
  };

  method = dummy;
  bool array = false;
  bool something_wrong = false;
  int  count = 0;       // for 'x' in "-Index x" or "-Random x"
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

  if( tok_vec.size() == 0 ){
    return CmdExec::errorOption( CMD_OPT_MISSING, "" );
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
          if( count < 0 ){
            error_msg_arr[i+1] = invalid__num;
          }
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
            if( count < 0 ){
              error_msg_arr[i+1] = invalid__num;
            }
            if( count == 0 ){
              error_msg_arr[i+1] = num_is_0;
            }
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

  if( mtest.getArrListSize() == 0 ){
    if( array && method == index )
      error_msg_arr[idx_tok_idx] = i_have_no_cash;
    else if( array && method == rndom )
      error_msg_arr[rn__tok_idx] = i_have_no_cash;
    something_wrong = true;
  }

  if( mtest.getObjListSize() == 0 ){
    if( method == index )
      error_msg_arr[idx_tok_idx] = i_have_no_cash;
    else if( array && method == rndom )
      error_msg_arr[rn__tok_idx] = i_have_no_cash;
    something_wrong = true;
  }


  if( method == index ){
    if( array ){
      if( mtest.getArrListSize() == 0 || mtest.getArrListSize()-1 < count){
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = index_idx_OoR;
      }
    }else{
      if( mtest.getObjListSize() == 0 || mtest.getObjListSize()-1 < count){
        something_wrong = true;
        error_msg_arr[cnt_tok_idx] = index_idx_OoR;
      }
    }
  }

  // third, check if there's error happened.
  for(size_t i = 0; i < tok_vec.size(); ++i){
    if( tok_vec[i] != "" ){
      error_msg_arr[i] = just_an_error;
    }
  }

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
    cerr << endl;
    for( size_t i = 0; i < error_msg_arr.size(); ++i ){
      cerr << _error_msg_Human[error_msg_arr[i]] << '\t';
      if( i%5 == 0 && i != 0 )
        cerr << endl;
    }
    cerr << endl;
#endif // MEM_DEBUG
    for( size_t i = 0; i < tok_vec_org.size(); ++i ){
      switch( error_msg_arr[i] ){ // now working....
        case okay:
          continue;
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
        case index_idx_OoR:
          if( method == array ){
            cerr << "Size of array list (" << mtest.getArrListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
          }else if( method == rndom ){
            cerr << "Size of object list (" << mtest.getObjListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
          }else{
#ifdef MEM_DEBUG
            assert(0 && "sth wrong in swithc case in MTDeleteCmd" );
#endif // MEM_DEBUG
          }
        case rndom_idx_OoR:
          if( method == array ){
            cerr << "Size of array list (" << mtest.getArrListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
          }else if( method == rndom ){
            cerr << "Size of object list (" << mtest.getObjListSize()
              << ") is <= " << count << "!!" << endl;
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, tok_vec_org[i] );
          }else{
#ifdef MEM_DEBUG
            assert(0 && "sth wrong in swithc case in MTDeleteCmd" );
#endif // MEM_DEBUG
          }
        case num_is_0:
          return CmdExec::errorOption( 
              CMD_OPT_ILLEGAL, tok_vec_org[cnt_tok_idx] );
        case invalid__num:
          return CmdExec::errorOption( CMD_OPT_ILLEGAL,
              tok_vec_org[i] );
        case i_have_no_cash:
          if( array )
            cerr << "Size of array list is " << mtest.getArrListSize()
              << "!!" << endl;
          else
            cerr << "size of object list is " << mtest.getObjListSize()
              << "!!" << endl;
          if( method == index )
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, 
                tok_vec_org[ idx_tok_idx ] );
          else if( method == rndom )
            return CmdExec::errorOption( CMD_OPT_ILLEGAL, 
                tok_vec_org[ rn__tok_idx ] );
          else
#ifdef MEM_DEBUG
            assert( 0 && "sth wrong in case i_have_no_cash" );
#endif // MEM_DEBUG
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
  if( method == rndom ){
    if( array ){
      for( int i = 0; i < count; ++i ){
        mtest.deleteArr( rnGen(mtest.getArrListSize() ) );
      }
    }
    else {
      for( int i = 0; i < count; ++i ){
        mtest.deleteObj( rnGen(mtest.getObjListSize() ) );
      }
    }
  }else if ( method == index ){
    assert( index >= 0 && "didn't exclude case (index < 0), = =." );
    if( array ){
      mtest.deleteArr( index );
    }else
      mtest.deleteObj( index );
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


