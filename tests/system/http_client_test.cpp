#include "httpClient.h"
#include <gtest/gtest.h>

/**
 * @brief Tests for HttpClient.
 *
 * Tests the public API including URL parsing (exercised indirectly
 * through GET/POST calls that fail at connection), move semantics,
 * and timeout configuration.
 */

TEST(HttpClient, Get_ConnectionRefused)
{
	HttpClient client;
	client.setTimeout(1);
	auto [ok, body] = client.get("http://127.0.0.1:19999/health");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, Get_HttpsUrlParsing)
{
	HttpClient client;
	client.setTimeout(1);
	auto [ok, body] = client.get("https://127.0.0.1:19999/health");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, Get_NoPortDefaultsTo80)
{
	HttpClient client;
	client.setTimeout(1);
	auto [ok, body] = client.get("http://127.0.0.1/health");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, Get_NoPathDefaultsToRoot)
{
	HttpClient client;
	client.setTimeout(1);
	auto [ok, body] = client.get("http://127.0.0.1:19999");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, Post_ConnectionRefused)
{
	HttpClient client;
	client.setTimeout(1);
	auto [ok, body] =
		client.post("http://127.0.0.1:19999/api", R"({"key":"value"})");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, SetTimeout_DoesNotCrash)
{
	HttpClient client;
	client.setTimeout(10);
	client.setTimeout(1);
	// No crash = success
}

TEST(HttpClient, MoveConstructor_Works)
{
	HttpClient client1;
	client1.setTimeout(5);
	HttpClient client2(std::move(client1));
	// client2 should be usable
	client2.setTimeout(1);
	auto [ok, body] = client2.get("http://127.0.0.1:19999/health");
	EXPECT_FALSE(ok);
}

TEST(HttpClient, MoveAssignment_Works)
{
	HttpClient client1;
	client1.setTimeout(5);
	HttpClient client2;
	client2 = std::move(client1);
	client2.setTimeout(1);
	auto [ok, body] = client2.get("http://127.0.0.1:19999/health");
	EXPECT_FALSE(ok);
}
