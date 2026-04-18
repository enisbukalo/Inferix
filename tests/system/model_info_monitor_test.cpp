#include <gtest/gtest.h>
#include "system/modelInfoMonitor.h"

// =============================================================================
// ModelInfoMonitor::parseMetricsResponse Tests
// =============================================================================

TEST(ModelInfoMonitor, ParseMetricsResponseZeroOnEmptyInput)
{
	auto info = ModelInfoMonitor::parseMetricsResponse("");
	EXPECT_DOUBLE_EQ(info.generationTokensPerSec, 0.0);
	EXPECT_DOUBLE_EQ(info.processingTokensPerSec, 0.0);
	EXPECT_EQ(info.totalPromptTokens, 0u);
	EXPECT_EQ(info.totalGenerationTokens, 0u);
}

TEST(ModelInfoMonitor, ParseMetricsResponseSkipsComments)
{
	std::string response =
		"# HELP llamacpp:predicted_tokens_seconds tokens/s\n"
		"# TYPE llamacpp:predicted_tokens_seconds gauge\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_DOUBLE_EQ(info.generationTokensPerSec, 0.0);
}

TEST(ModelInfoMonitor, ParseMetricsResponseGenerationTokPerSec)
{
	std::string response =
		"llamacpp:predicted_tokens_seconds{model=\"test\"} 67.7964\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_DOUBLE_EQ(info.generationTokensPerSec, 67.7964);
}

TEST(ModelInfoMonitor, ParseMetricsResponseProcessingTokPerSec)
{
	std::string response =
		"llamacpp:prompt_tokens_seconds{model=\"test\"} 1664.81\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_DOUBLE_EQ(info.processingTokensPerSec, 1664.81);
}

TEST(ModelInfoMonitor, ParseMetricsResponseTotalPromptTokens)
{
	std::string response =
		"llamacpp:prompt_tokens_total{model=\"test\"} 98986\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_EQ(info.totalPromptTokens, 98986u);
}

TEST(ModelInfoMonitor, ParseMetricsResponseTotalGenerationTokens)
{
	std::string response =
		"llamacpp:tokens_predicted_total{model=\"test\"} 6829\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_EQ(info.totalGenerationTokens, 6829u);
}

TEST(ModelInfoMonitor, ParseMetricsResponseAllFieldsFromRealResponse)
{
	std::string response =
		"# HELP llamacpp:prompt_tokens_total total prompt tokens\n"
		"# TYPE llamacpp:prompt_tokens_total counter\n"
		"llamacpp:prompt_tokens_total{model=\"Qwen\"} 98986\n"
		"# HELP llamacpp:tokens_predicted_total total generated tokens\n"
		"# TYPE llamacpp:tokens_predicted_total counter\n"
		"llamacpp:tokens_predicted_total{model=\"Qwen\"} 6829\n"
		"# HELP llamacpp:prompt_tokens_seconds prompt throughput tok/s\n"
		"# TYPE llamacpp:prompt_tokens_seconds gauge\n"
		"llamacpp:prompt_tokens_seconds{model=\"Qwen\"} 1664.81\n"
		"# HELP llamacpp:predicted_tokens_seconds generation throughput tok/s\n"
		"# TYPE llamacpp:predicted_tokens_seconds gauge\n"
		"llamacpp:predicted_tokens_seconds{model=\"Qwen\"} 67.7964\n"
		"# Unrelated metric should be ignored\n"
		"llamacpp:tokens_predicted_seconds_total{model=\"Qwen\"} 100.728\n";

	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_DOUBLE_EQ(info.totalPromptTokens, 98986u);
	EXPECT_DOUBLE_EQ(info.totalGenerationTokens, 6829u);
	EXPECT_DOUBLE_EQ(info.processingTokensPerSec, 1664.81);
	EXPECT_DOUBLE_EQ(info.generationTokensPerSec, 67.7964);
}

TEST(ModelInfoMonitor, ParseMetricsResponseIgnoresOldUnprefixedNames)
{
	// Old broken metric names (without llamacpp: prefix) must not match
	std::string response =
		"predicted_tokens_seconds{model=\"test\"} 99.9\n"
		"prompt_tokens_seconds{model=\"test\"} 888.0\n"
		"prompt_tokens_total{model=\"test\"} 5000\n"
		"tokens_predicted_total{model=\"test\"} 1000\n";
	auto info = ModelInfoMonitor::parseMetricsResponse(response);
	EXPECT_DOUBLE_EQ(info.generationTokensPerSec, 0.0);
	EXPECT_DOUBLE_EQ(info.processingTokensPerSec, 0.0);
	EXPECT_EQ(info.totalPromptTokens, 0u);
	EXPECT_EQ(info.totalGenerationTokens, 0u);
}
