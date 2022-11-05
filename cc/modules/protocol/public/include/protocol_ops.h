// Copyright 2020 The LatticeX Foundation
// This file is part of the Rosetta library.
//
// The Rosetta library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Rosetta library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Rosetta library. If not, see <http://www.gnu.org/licenses/>.
// ==============================================================================
#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <map>
#include <stdexcept>
#include <memory>
#include <vector>
#include "cc/modules/protocol/mpc/comm/include/mpc_defines.h"
#include "cc/modules/common/include/utils/msg_id.h"

namespace rosetta {

using std::cout;
using std::endl;
using std::map;
using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;
using attr_type = unordered_map<string, string>;

#define THROW_NOT_IMPL      \
  throw std::runtime_error( \
    string("please implements the interface " + string(__FUNCTION__) + string(" in subclass")))
#define THROW_NOT_IMPL_FN(name) \
  throw std::runtime_error(string("please implements '") + name + "' in subclass")

struct SaverModel {
 private:
  enum mode {
    SAVER_MODE_INVALID = 0,
    SAVER_MODE_LOCAL_CIPHERTEXT = 1,           // computation nodes save ciphertext model locally
    SAVER_MODE_CIPHERTEXT = 2,                 // computation nodes or result nodes save ciphertext model
    SAVER_MODE_PLAINTEXT = 3                   //  result nodes save plaintext model
  };

  enum mode mode_ = SAVER_MODE_INVALID;
  map<string, vector<string>> ciphertext_nodes_;
  vector<string> plaintext_nodes_;

  void clear() {
    switch (mode_) {
      case SAVER_MODE_CIPHERTEXT:
        ciphertext_nodes_.clear();
        break;
      case SAVER_MODE_PLAINTEXT:
        plaintext_nodes_.clear();
        break;
      default:
        break;
    }
    mode_ = SAVER_MODE_INVALID;
  }

 public:
  void set_local_ciphertext_mode() {
    clear();
    mode_ = SAVER_MODE_LOCAL_CIPHERTEXT;
  }

  void set_ciphertext_mode(const map<string, vector<string>>& ciphertext_nodes) {
    clear();
    mode_ = SAVER_MODE_CIPHERTEXT;
    ciphertext_nodes_ = ciphertext_nodes;
  }

  void set_plaintext_mode(const vector<string>& plaintext_nodes) {
    clear();
    mode_ = SAVER_MODE_PLAINTEXT;
    plaintext_nodes_ = plaintext_nodes;
  }

  bool is_local_ciphertext_mode() const { return mode_ == SAVER_MODE_LOCAL_CIPHERTEXT; }

  bool is_ciphertext_mode() const { return mode_ == SAVER_MODE_CIPHERTEXT; }

  bool is_plaintext_mode() const { return mode_ == SAVER_MODE_PLAINTEXT; }

  const map<string, vector<string>>& get_ciphertext_nodes() const { return ciphertext_nodes_; }

  const vector<string>& get_plaintext_nodes() const { return plaintext_nodes_; }
};

struct RestoreModel {
 private:
  enum mode {
    RESTORE_MODE_INVALID = 0,
    RESTORE_MODE_LOCAL_CIPHERTEXT = 1,            // computation nodes restore ciphertext model locally
    RESTORE_MODE_CIPHERTEXT = 2,                  // computation nodes or data nodes restore ciphertext model
    RESTORE_MODE_PLAINTEXT = 3,                   // data node restore plaintext
    RESTORE_MODE_LOCAL_PLAINTEXT = 4              // computation nodes restore plaintext locally
  };

  enum mode mode_ = RESTORE_MODE_INVALID;
  map<string, string> ciphertext_nodes_;
  string plaintext_node_;

  void clear() {
    switch (mode_) {
      case RESTORE_MODE_CIPHERTEXT:
        ciphertext_nodes_.clear();
        break;
      case RESTORE_MODE_PLAINTEXT:
        plaintext_node_.clear();
        break;
      default:
        break;
    }
    mode_ = RESTORE_MODE_INVALID;
  }

 public:
  void set_local_ciphertext_mode() {
    clear();
    mode_ = RESTORE_MODE_LOCAL_CIPHERTEXT;
  }
  void set_ciphertext_mode(const map<string, string>& ciphertext_nodes) {
    clear();
    mode_ = RESTORE_MODE_CIPHERTEXT;
    ciphertext_nodes_ = ciphertext_nodes;
  }
  void set_plaintext_mode(const string& plaintext_node) {
    clear();
    mode_ = RESTORE_MODE_PLAINTEXT;
    plaintext_node_ = plaintext_node;
  }
  void set_local_plaintext_mode() {
    clear();
    mode_ = RESTORE_MODE_LOCAL_PLAINTEXT;
  }

  bool is_local_ciphertext_mode() const { return mode_ == RESTORE_MODE_LOCAL_CIPHERTEXT; }
  bool is_ciphertext_mode() const { return mode_ == RESTORE_MODE_CIPHERTEXT; }
  bool is_plaintext_mode() const { return mode_ == RESTORE_MODE_PLAINTEXT; }
  bool is_local_plaintext_mode() const { return mode_ == RESTORE_MODE_LOCAL_PLAINTEXT; }

  const map<string, string>& get_ciphertext_nodes() const { return ciphertext_nodes_; }
  const string& get_plaintext_node() const { return plaintext_node_; }
};

struct ProtocolContext {
  short VERSION = 2;
  int FLOAT_PRECISION = FLOAT_PRECISION_DEFAULT;

  // To specify nodes to save model
  SaverModel SAVER_MODEL;
  // To specify nodes to load or restore model
  RestoreModel RESTORE_MODEL;
  string TASK_ID;
  string NODE_ID;
  int ROLE_ID;
  map<string, int> NODE_ROLE_MAPPING; // node_id -> role
  string PAYLOAD;

  int GetMyRole() { return ROLE_ID; }

  int GetRole(const string& node_id) {
    if (NODE_ROLE_MAPPING.find(node_id) == NODE_ROLE_MAPPING.end()) {
      std::cerr << "cannot find node_id: " << node_id << " when get role id!" << std::endl;
      return -1;
    }

    return NODE_ROLE_MAPPING[node_id];
  }

  string GetNodeId(const int role) {
    for (auto iter = NODE_ROLE_MAPPING.begin(); iter != NODE_ROLE_MAPPING.end(); ++iter) {
      if (iter->second == role)
        return iter->first;
    }
    std::cerr << "cannot find node_id for role id: " << role << std::endl;
    return "";
  }
};

// This is the interface that each specific cryptographic protocol should implement!
class ProtocolOps {
 public:
  /**
   * message id for parallel communication
   */
  msg_id_t _op_msg_id;

  /**
   * protocol execution context to support multiple tasks execute in parallel .
   */
  shared_ptr<ProtocolContext> context_;

 public:
  const msg_id_t& msg_id() const { return _op_msg_id; }
  ProtocolOps(const msg_id_t& msg_id, shared_ptr<ProtocolContext> context)
      : _op_msg_id(msg_id), context_(context){};

  virtual ~ProtocolOps() = default;

  /**
   * @desc: encode the literal number to a protocol-specific format 
   *         wrapped as string.
   * @param:
   *     in, the vector of literal number
   *     out, the vector of the encoded strings
   * @return:
   *     0 if success, errcode otherwise
   */
  virtual int TfToSecure(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // Decode the string in protocol-specific format to literal number
  virtual int SecureToTf(
    const vector<string>& in,
    vector<string>& out,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int RandSeed(std::string op_seed, string& out_str) { THROW_NOT_IMPL; }
  virtual uint64_t RandSeed() {
    std::cerr << "insecure seed!!! please overide " << __FUNCTION__ << std::endl;
    return 0x123456;
  }
  virtual uint64_t RandSeed(vector<uint64_t>& seed) {
    std::cerr << "insecure seed!!! please overide " << __FUNCTION__ << std::endl;
    return 0x123456;
  }

  virtual int PrivateInput(
    const string& node_id,
    const vector<double>& in_x,
    vector<string>& out_x) {
    THROW_NOT_IMPL;
  }
  virtual int PublicInput(
    const string& node_id,
    const vector<double>& in_x,
    vector<string>& out_x) {
    THROW_NOT_IMPL;
  }

  virtual int Broadcast(const string& from_node, const string& msg, string& result) {
    THROW_NOT_IMPL;
  }

  // result must be allocate outside!
  virtual int Broadcast(const string& from_node, const char* msg, char* result, size_t size) {
    THROW_NOT_IMPL;
  }

  /**
   * @desc: This is for Tensorflow's SaveV2 Op.
   */
  virtual int ConditionalReveal(
    vector<string>& in_vec,
    vector<string>& out_cipher_vec,
    vector<double>& out_plain_vec) {
    THROW_NOT_IMPL;
  }

  //////////////////////////////////    math ops   //////////////////////////////////
  virtual int Add(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr);

  virtual int Sub(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Mul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Div(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Reciprocaldiv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Truediv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Floordiv(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  //// compare ops ////
  virtual int Less(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int LessEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Equal(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int NotEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Greater(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int GreaterEqual(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  //// compare ////

  virtual int Pow(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Matmul(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Square(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Negative(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Abs(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  /**
   * @desc: get the derivative of |X| function by using MSB.
   * @note: you may override this in your derived class to have a customized algorithm
   */
  virtual int AbsPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    cout << "TODO: base AbsPrime called" << endl;
    return 0;
  }

  virtual int Log(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Log1p(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  /**
   * @desc: optional, for high-precision logarithm. 
   *    The default implementation in base class will use the Log.
   */
  virtual int HLog(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    return Log(a, output, attr_info);
  }

  virtual int Max(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Min(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Mean(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Sum(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Sum(const vector<string>& a, string& output, const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int AddN(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  ////////////////////////////////// nn ops //////////////////////////////////
  virtual int Relu(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int ReluPrime(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    cout << "TODO: base ReluPrime!" << endl;
    return 0;
  }

  virtual int Sigmoid(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int SigmoidCrossEntropy(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int BiasAdd(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // get \sqrt{a}
  virtual int Sqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // get 1/\sqrt{a}
  virtual int Rsqrt(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Invert(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Exp(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  ////////////////////////////////// training ops //////////////////////////////////
  virtual int Reveal(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Reveal(
    const vector<string>& a,
    vector<double>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  ////////////////////////////////// logical ops //////////////////////////////////
  virtual int AND(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int OR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int XOR(
    const vector<string>& a,
    const vector<string>& b,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }
  virtual int NOT(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  virtual int Softmax(
    const vector<string>& a,
    vector<string>& output,
    const attr_type* attr_info = nullptr) {
    THROW_NOT_IMPL;
  }

  // Sort & Permutation. Only supports A.size() = cols*(2^k)
  /**
   * \param[in,out] A, a vector, the default sorted order is ascending
   * \param[in] attr, 'descending'. indicates the sorted order, the default value is 0
   */
  virtual int Sort(vector<string>& A, const attr_type* attr = nullptr) { THROW_NOT_IMPL; }
  /**
   * \param[in,out] A, a vector, the default sorted order is ascending
   * \param[out] R, a vector, indicates whether to swap two elements
   * \param[in] attr, 'descending'. indicates the sorted order, the default value is 0
   */
  virtual int Sort(vector<string>& A, vector<string>& R, const attr_type* attr = nullptr) {
    THROW_NOT_IMPL;
  }
  /**
   * \param[in,out] A, a matrix of shape rows x cols, which passed by `attr`, row first
   * \param[in] R, a vector, indicates whether to swap two elements
   * \param[in] attr, 'cols'. the default value is 1
   */
  virtual int Permutation(
    vector<string>& A,
    const vector<string>& R,
    const attr_type* attr = nullptr) {
    THROW_NOT_IMPL;
  }
  /**
   * \param[in,out] A, a matrix of shape rows x cols, which passed by `attr`, row first
   * \param[in] R, a vector, indicates whether to swap two elements
   * \param[in] attr, 'cols'. the default value is 1
   */
  virtual int InversePermutation(
    vector<string>& A,
    const vector<string>& R,
    const attr_type* attr = nullptr) {
    THROW_NOT_IMPL;
  }
};

} // namespace rosetta
