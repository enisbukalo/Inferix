#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <utility>

/**
 * @class HttpClient
 * @brief Minimal HTTP client for llama-server API calls.
 *
 * Thread-safe, uses httplib library under the hood.
 */
class HttpClient
{
  public:
	HttpClient() noexcept;
	~HttpClient();

	// Non-copyable
	HttpClient(const HttpClient &) = delete;
	HttpClient &operator=(const HttpClient &) = delete;

	// Movable
	HttpClient(HttpClient &&) noexcept;
	HttpClient &operator=(HttpClient &&) noexcept;

	/**
	 * @brief Perform GET request.
	 * @param url Full URL (e.g., "http://127.0.0.1:8080/health")
	 * @return Pair of (success, response body or error message)
	 */
	std::pair<bool, std::string> get(std::string_view url);

	/**
	 * @brief Perform POST request with JSON body.
	 * @param url Full URL
	 * @param jsonBody JSON payload
	 * @return Pair of (success, response body or error message)
	 */
	std::pair<bool, std::string> post(std::string_view url,
									  std::string_view jsonBody);

	/**
	 * @brief Set the timeout in seconds.
	 * @param timeoutSeconds Timeout value
	 */
	void setTimeout(int timeoutSeconds);

  private:
	class Impl;
	std::unique_ptr<Impl> m_impl;
};