#include "httpClient.h"
#include "server/httplib.h"
#include <chrono>
#include <spdlog/spdlog.h>
#include <utility>

class HttpClient::Impl
{
  public:
	Impl() : m_client("127.0.0.1", 80), m_timeout(5)
	{
	}

	std::pair<bool, std::string> get(std::string_view url)
	{
		try {
			// Create client for this request (host extracted from URL)
			auto [host, port, path] = parseUrl(url);
			if (host.empty()) {
				return { false, "Invalid URL: could not parse host" };
			}

			httplib::Client cli(host.c_str(), port);
			cli.set_connection_timeout(m_timeout);
			cli.set_read_timeout(m_timeout);

			auto response = cli.Get(path.data());

			if (!response || response->status != 200) {
				std::string err =
					response ? "HTTP " + std::to_string(response->status)
							 : "No response";
				spdlog::debug("HTTP GET {} failed: {}", url, err);
				return { false, err };
			}

			return { true, response->body };
		} catch (const std::exception &e) {
			spdlog::debug("HTTP GET {} exception: {}", url, e.what());
			return { false, e.what() };
		}
	}

	std::pair<bool, std::string> post(std::string_view url,
									  std::string_view jsonBody)
	{
		try {
			auto [host, port, path] = parseUrl(url);
			if (host.empty()) {
				return { false, "Invalid URL: could not parse host" };
			}

			httplib::Client cli(host.c_str(), port);
			cli.set_connection_timeout(m_timeout);
			cli.set_read_timeout(m_timeout);
			cli.set_write_timeout(m_timeout);

			auto response =
				cli.Post(path.data(), jsonBody.data(), "application/json");

			if (!response ||
				(response->status != 200 && response->status != 201)) {
				std::string err =
					response ? "HTTP " + std::to_string(response->status)
							 : "No response";
				spdlog::debug("HTTP POST {} failed: {}", url, err);
				return { false, err };
			}

			return { true, response->body };
		} catch (const std::exception &e) {
			spdlog::debug("HTTP POST {} exception: {}", url, e.what());
			return { false, e.what() };
		}
	}

	void setTimeout(int timeoutSeconds)
	{
		m_timeout = timeoutSeconds;
	}

  private:
	// Simple URL parser - extracts host, port, and path from full URL
	// Expected format: http://host:port/path or http://host/path
	static std::tuple<std::string, int, std::string>
	parseUrl(std::string_view url)
	{
		std::string host;
		int port = 80;
		std::string path = "/";

		// Skip "http://"
		if (url.starts_with("http://")) {
			url = url.substr(7);
		} else if (url.starts_with("https://")) {
			url = url.substr(8);
			port = 443;
		}

		// Find first / to separate host:port from path
		auto slashPos = url.find('/');
		if (slashPos == std::string_view::npos) {
			host = std::string(url);
			path = "/";
		} else {
			host = std::string(url.substr(0, slashPos));
			path = std::string(url.substr(slashPos));
		}

		// Extract port from host if present (e.g., "127.0.0.1:8080")
		auto colonPos = host.rfind(':');
		if (colonPos != std::string::npos) {
			// Check if what follows colon is digits (port number)
			bool isPort = true;
			for (size_t i = colonPos + 1; i < host.size(); ++i) {
				if (!std::isdigit(host[i])) {
					isPort = false;
					break;
				}
			}
			if (isPort) {
				std::string portStr = host.substr(colonPos + 1);
				host = host.substr(0, colonPos);
				try {
					port = std::stoi(portStr);
				} catch (...) {
					port = 80;
				}
			}
		}

		return { host, port, path };
	}

	httplib::Client m_client;
	int m_timeout;
};

// Implementation of HttpClient methods
HttpClient::HttpClient() noexcept : m_impl(std::make_unique<Impl>())
{
}

HttpClient::~HttpClient() = default;

HttpClient::HttpClient(HttpClient &&) noexcept = default;
HttpClient &HttpClient::operator=(HttpClient &&) noexcept = default;

std::pair<bool, std::string> HttpClient::get(std::string_view url)
{
	return m_impl->get(url);
}

std::pair<bool, std::string> HttpClient::post(std::string_view url,
											  std::string_view jsonBody)
{
	return m_impl->post(url, jsonBody);
}

void HttpClient::setTimeout(int timeoutSeconds)
{
	m_impl->setTimeout(timeoutSeconds);
}