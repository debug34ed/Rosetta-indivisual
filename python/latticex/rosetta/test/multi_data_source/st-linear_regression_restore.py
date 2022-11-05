#!/usr/bin/env python3
import latticex.rosetta as rtt  # difference from tensorflow
import math
import os
import csv
import tensorflow as tf
import numpy as np
from util import read_dataset

np.set_printoptions(suppress=True)

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'

np.random.seed(0)

EPOCHES = 10
BATCH_SIZE = 16
learning_rate = 0.0002

task_id = 'task-id'
rtt.py_protocol_handler.set_loglevel(0)
rtt.activate("Helix", task_id = task_id)
rtt.set_restore_model(['p0','p1','p2'], task_id = task_id)
#rtt.set_restore_model(['p0', 'p1', 'p2'])
node_id = rtt.get_current_node_id(task_id = task_id)

# real data
# ######################################## difference from tensorflow
file_x = '../dsets/' + node_id + "/reg_test_x.csv"
file_y = '../dsets/' + node_id + "/reg_test_y.csv"
real_X, real_Y = rtt.PrivateDataset(data_owner=(
    0, 'p9'), label_owner=1, task_id = task_id).load_data(file_x, file_y, header=None)
# ######################################## difference from tensorflow
DIM_NUM = real_X.shape[1]

X = tf.placeholder(tf.float64, [None, DIM_NUM])
Y = tf.placeholder(tf.float64, [None, 1])
print(X)
print(Y)

# initialize W & b
W = tf.Variable(tf.zeros([DIM_NUM, 1], dtype=tf.float64))
b = tf.Variable(tf.zeros([1], dtype=tf.float64))
print(W)
print(b)

# predict
pred_Y = tf.matmul(X, W) + b
print(pred_Y)


# save
saver = tf.train.Saver(var_list=None, max_to_keep=5, name='v2')
os.makedirs("./log/ckpt_" + node_id, exist_ok=True)

# init
init = tf.global_variables_initializer()
reveal_Y = rtt.SecureReveal(pred_Y, receive_party=[0, 1, 2, 'p9'])
with tf.Session(task_id = task_id) as sess:
    sess.run(init)
    if os.path.exists('./log/ckpt_' + node_id +'/checkpoint'):
        saver.restore(sess, './log/ckpt_' + node_id + '/model')
    print('X:', X)
    print('W:', W)

    # predict
    Y_pred = sess.run(pred_Y, feed_dict={X: real_X})
    print("Y_pred:", Y_pred)

    reveal_y = sess.run(reveal_Y, feed_dict={X: real_X})
    print("reveal_Y:", reveal_y)

print(rtt.get_perf_stats(True, task_id = task_id))
rtt.deactivate(task_id = task_id)
