#!/usr/bin/python

import tensorflow as tf
import numpy as np
np.set_printoptions(suppress=True)

import latticex.rosetta as cb
import grad_test_utils as common

res_flag = True
sess = None


def test_sum_grad(X, axis, out_g, protocol="Helix"):
    cb.activate(protocol)

    global sess
    if sess is not None:
        sess.close()

    # ===========================
    # init global var
    # ===========================
    init = tf.compat.v1.global_variables_initializer()
    sess = tf.compat.v1.Session()
    sess.run(init)

    
    # ===========================
    # run mpc sum grad
    # ===========================
    print("===========================")
    print("# run mpc sum(X) grad, axis=", axis)
    mpc_Y = cb.SecureSum(X, axis=axis)
    mpc_g = tf.gradients(mpc_Y, [common.get_var_from_rtt_tensor(X)])
    print(sess.run(mpc_g))
    print("===========================")
    
    
    # ===========================
    # check mpcsum grads value
    # ===========================
    mpc_out_g = []
    for i in range(len(mpc_g)):
        print("---------- Reveal mpcsum grad ------------")
        mpc_out_g.append(sess.run(cb.SecureReveal(mpc_g[i])))
        print(mpc_out_g)
        print("------------------------------------------")

    global res_flag
    res_flag = res_flag and common.check_mpc_op_grads(out_g, mpc_out_g)
    


#===========================
# define tf varables
#===========================
X = tf.Variable([0.77])  # 1 x 1
X2 = tf.Variable([[21.34, -11.43], [6.291, 6.311]])  # 2 x 2
X3 = tf.Variable([[21.34, 21.34], [6.291, 6.291]])   # 2 x 2



#===========================
# run case
#===========================
# test helix grad op
print("run helix protocol...")
test_sum_grad(X, None, [1.0])
test_sum_grad(X2, None, [[1., 1.], [1., 1.]])
test_sum_grad(X2, 0, [[1., 1.], [1., 1.]])
test_sum_grad(X2, 1, [[1., 1.], [1., 1.]])
test_sum_grad(X2, [0,1], [[1., 1.], [1., 1.]])
test_sum_grad(X3, None, [[1., 1.], [1., 1.]])
test_sum_grad(X3, 0, [[1., 1.], [1., 1.]])
test_sum_grad(X3, 1, [[1., 1.], [1., 1.]])
test_sum_grad(X3, [0,1], [[1., 1.], [1., 1.]])

# test snn grad op
print("run snn protocol...")
test_sum_grad(X, None, [1.0], protocol="SecureNN")
test_sum_grad(X2, None, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X2, 0, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X2, 1, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X2, [0,1], [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X3, None, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X3, 0, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X3, 1, [[1., 1.], [1., 1.]], protocol="SecureNN")
test_sum_grad(X3, [0,1], [[1., 1.], [1., 1.]], protocol="SecureNN")


#===========================
# pass or fail
#===========================
common.print_check_result(res_flag)



Writer = tf.summary.FileWriter("log", tf.get_default_graph())
Writer.close()
cb.deactivate()