/****************************************************************************
 *   Copyright (c) 2015 James Wilson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name ATLFlight nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dspal_signal.h>
#include <pthread.h>

#include "test_utils.h"
#include "dspal_tester.h"

#define SKIP_PTHREAD_KILL
#define DSPAL_TESTER_COND_WAIT_TIMEOUT_IN_SECS 3

class PthreadTest
{
public:
	PthreadTest();
	~PthreadTest();

	int doTests();

private:
	int createAndJoin(void *(*helper)(void *), void *test_var);
	int testInit();
	int testCreate();
	int testSelf();
	int testExit();

	pthread_t m_tid;

	int m_init_test[10*2+1];
};

static PthreadTest test1;

PthreadTest::PthreadTest()
{
	for (int i=-10; i<=10; ++i)
	{
		m_init_test[i] = 10-i;
	}
}

PthreadTest::~PthreadTest()
{
}

int PthreadTest::testInit()
{
	for (int i=-10; i<=10; ++i)
	{
		if (m_init_test[i] != 10-i) FAIL("incorrect initialization value");
	}
	return TEST_PASS;
}

int PthreadTest::doTests()
{
	if (testInit() != TEST_PASS) FAIL("C++ init test failed");
	if (testCreate() != TEST_PASS) FAIL("pthread_reate test failed");
	if (testSelf() != TEST_PASS) FAIL("pthread_self test failed");
	if (testExit() != TEST_PASS) FAIL("pthread_exit test failed");

	return TEST_PASS;
}

int PthreadTest::createAndJoin(void *(*helper)(void *), void *test_var)
{
	int rv = pthread_create(&m_tid, NULL, helper, &test_var);
	if (rv != 0) FAIL("thread_create returned error");

	rv = pthread_join(m_tid, NULL);
	if (rv != 0) FAIL("thread_join returned error");

	return TEST_PASS;
}

static void *createHelper(void *test_value)
{
	int *v = (int*)test_value;
	(*v) = 1;

	return NULL;
}

int PthreadTest::testCreate(void)
{
	int test_value = 0;

	int rv = createAndJoin(createHelper, &test_value);

	if (rv == TEST_PASS && test_value != 1) FAIL("test value did not change");

	return rv;
}

static void *selfHelper(void *thread_self)
{
	pthread_t *v = (pthread_t*)thread_self;
	(*v) = pthread_self();

	return NULL;
}

int PthreadTest::testSelf(void)
{
	pthread_t thread_self;

	int rv = createAndJoin(selfHelper, &thread_self);

	if (rv == TEST_PASS && thread_self != m_tid) FAIL("pthread_self did not return the expected value");

	return rv;
}

static void *exitHelper(void *test_value)
{
	pthread_exit(NULL);

	int *v = (int*)test_value;
	(*v) = 1;

	return NULL;
}

int PthreadTest::testExit(void)
{
	int test_value = 0;

	int rv = createAndJoin(exitHelper, &test_value);

	if (rv == TEST_PASS && test_value != 0) FAIL("test value should not have changed");

	return rv;
}

// Implementation of DSP side of IDL interface spec
int dspal_tester_test_cxx_static()
{
	int rv = test1.doTests();

	if (rv != TEST_PASS) FAIL("static initialized PthreadTest failed");

	return TEST_PASS;
}

int dspal_tester_test_cxx_heap()
{
	PthreadTest *test2 = new PthreadTest;

	int rv = test2->doTests();

	delete test2;
	if (rv != TEST_PASS) FAIL("heap allocated PthreadTest failed");

	return rv;
}

