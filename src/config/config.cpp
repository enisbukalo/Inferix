/**
 * @file config.cpp
 * @brief JSON serialization functions for configuration structures.
 *
 * Implements to_json/from_json functions for all Config structures
 * using nlohmann::json. These functions enable automatic serialization
 * and deserialization of the UserConfig structure.
 *
 * The serialization format is:
 * @code
 * {
 *   "server": { ... },
 *   "load": { ... },
 *   "inference": { ... },
 *   "ui": { ... },
 *   "terminal": { ... },
 *   "presets": [ ... ]
 * }
 * @endcode
 */

#include "config.h"

using json = nlohmann::json;

namespace Config {

// ============================================================================
// ServerSettings serialization
// ============================================================================

void to_json(json &j, const ServerSettings &v)
{
	j["host"] = v.host;
	j["port"] = v.port;
	j["api_key"] = v.api_key;
	j["api_key_file"] = v.api_key_file;
	j["timeout"] = v.timeout;
	j["threads_http"] = v.threads_http;

	j["ssl_key_file"] = v.ssl_key_file;
	j["ssl_cert_file"] = v.ssl_cert_file;

	j["path"] = v.path;
	j["api_prefix"] = v.api_prefix;
	j["media_path"] = v.media_path;

	j["alias"] = v.alias;
	j["webui"] = v.webui;
	j["embedding"] = v.embedding;
	j["reranking"] = v.reranking;
	j["cont_batching"] = v.cont_batching;
	j["cache_prompt"] = v.cache_prompt;
	j["cache_reuse"] = v.cache_reuse;
	j["context_shift"] = v.context_shift;
	j["warmup"] = v.warmup;
	j["jinja"] = v.jinja;
	j["prefill_assistant"] = v.prefill_assistant;
	j["slot_prompt_similarity"] = v.slot_prompt_similarity;
	j["sleep_idle_seconds"] = v.sleep_idle_seconds;

	j["metrics"] = v.metrics;
	j["props"] = v.props;
	j["slots"] = v.slots;
	j["slot_save_path"] = v.slot_save_path;
}

void from_json(const json &j, ServerSettings &v)
{
	v.host = j.value("host", v.host);
	v.port = j.value("port", v.port);
	v.api_key = j.value("api_key", v.api_key);
	v.api_key_file = j.value("api_key_file", v.api_key_file);
	v.timeout = j.value("timeout", v.timeout);
	v.threads_http = j.value("threads_http", v.threads_http);

	v.ssl_key_file = j.value("ssl_key_file", v.ssl_key_file);
	v.ssl_cert_file = j.value("ssl_cert_file", v.ssl_cert_file);

	v.path = j.value("path", v.path);
	v.api_prefix = j.value("api_prefix", v.api_prefix);
	v.media_path = j.value("media_path", v.media_path);

	v.alias = j.value("alias", v.alias);
	v.webui = j.value("webui", v.webui);
	v.embedding = j.value("embedding", v.embedding);
	v.reranking = j.value("reranking", v.reranking);
	v.cont_batching = j.value("cont_batching", v.cont_batching);
	v.cache_prompt = j.value("cache_prompt", v.cache_prompt);
	v.cache_reuse = j.value("cache_reuse", v.cache_reuse);
	v.context_shift = j.value("context_shift", v.context_shift);
	v.warmup = j.value("warmup", v.warmup);
	v.jinja = j.value("jinja", v.jinja);
	v.prefill_assistant = j.value("prefill_assistant", v.prefill_assistant);
	v.slot_prompt_similarity =
		j.value("slot_prompt_similarity", v.slot_prompt_similarity);
	v.sleep_idle_seconds = j.value("sleep_idle_seconds", v.sleep_idle_seconds);

	v.metrics = j.value("metrics", v.metrics);
	v.props = j.value("props", v.props);
	v.slots = j.value("slots", v.slots);
	v.slot_save_path = j.value("slot_save_path", v.slot_save_path);
}

// ============================================================================
// LoadSettings serialization
// ============================================================================

void to_json(json &j, const LoadSettings &v)
{
	j["model_path"] = v.model_path;
	j["model_url"] = v.model_url;
	j["hf_repo"] = v.hf_repo;
	j["hf_file"] = v.hf_file;
	j["hf_token"] = v.hf_token;

	j["n_gpu_layers"] = v.n_gpu_layers;
	j["split_mode"] = v.split_mode;
	j["tensor_split"] = v.tensor_split;
	j["main_gpu"] = v.main_gpu;

	j["ctx_size"] = v.ctx_size;
	j["batch_size"] = v.batch_size;
	j["ubatch_size"] = v.ubatch_size;
	j["parallel"] = v.parallel;

	j["cache_type_k"] = v.cache_type_k;
	j["cache_type_v"] = v.cache_type_v;
	j["kv_offload"] = v.kv_offload;

	j["flash_attn"] = v.flash_attn;
	j["mlock"] = v.mlock;
	j["mmap"] = v.mmap;

	j["threads"] = v.threads;
	j["threads_batch"] = v.threads_batch;

	j["lora"] = v.lora;
	j["mmproj"] = v.mmproj;

	j["model_draft"] = v.model_draft;
	j["draft_max"] = v.draft_max;

	j["chat_template"] = v.chat_template;
	j["reasoning_format"] = v.reasoning_format;

	j["fit"] = v.fit;
}

void from_json(const json &j, LoadSettings &v)
{
	v.model_path = j.value("model_path", v.model_path);
	v.model_url = j.value("model_url", v.model_url);
	v.hf_repo = j.value("hf_repo", v.hf_repo);
	v.hf_file = j.value("hf_file", v.hf_file);
	v.hf_token = j.value("hf_token", v.hf_token);

	v.n_gpu_layers = j.value("n_gpu_layers", v.n_gpu_layers);
	v.split_mode = j.value("split_mode", v.split_mode);
	v.tensor_split = j.value("tensor_split", v.tensor_split);
	v.main_gpu = j.value("main_gpu", v.main_gpu);

	v.ctx_size = j.value("ctx_size", v.ctx_size);
	v.batch_size = j.value("batch_size", v.batch_size);
	v.ubatch_size = j.value("ubatch_size", v.ubatch_size);
	v.parallel = j.value("parallel", v.parallel);

	v.cache_type_k = j.value("cache_type_k", v.cache_type_k);
	v.cache_type_v = j.value("cache_type_v", v.cache_type_v);
	v.kv_offload = j.value("kv_offload", v.kv_offload);

	v.flash_attn = j.value("flash_attn", v.flash_attn);
	v.mlock = j.value("mlock", v.mlock);
	v.mmap = j.value("mmap", v.mmap);

	v.threads = j.value("threads", v.threads);
	v.threads_batch = j.value("threads_batch", v.threads_batch);

	v.lora = j.value("lora", v.lora);
	v.mmproj = j.value("mmproj", v.mmproj);

	v.model_draft = j.value("model_draft", v.model_draft);
	v.draft_max = j.value("draft_max", v.draft_max);

	v.chat_template = j.value("chat_template", v.chat_template);
	v.reasoning_format = j.value("reasoning_format", v.reasoning_format);

	v.fit = j.value("fit", v.fit);
}

// ============================================================================
// InferenceSettings serialization
// ============================================================================

void to_json(json &j, const InferenceSettings &v)
{
	j["n_predict"] = v.n_predict;
	j["samplers"] = v.samplers;

	j["seed"] = v.seed;
	j["temperature"] = v.temperature;
	j["top_k"] = v.top_k;
	j["top_p"] = v.top_p;
	j["min_p"] = v.min_p;
	j["top_nsigma"] = v.top_nsigma;
	j["typical_p"] = v.typical_p;

	j["xtc_probability"] = v.xtc_probability;
	j["xtc_threshold"] = v.xtc_threshold;

	j["repeat_last_n"] = v.repeat_last_n;
	j["repeat_penalty"] = v.repeat_penalty;
	j["presence_penalty"] = v.presence_penalty;
	j["frequency_penalty"] = v.frequency_penalty;

	j["dry_multiplier"] = v.dry_multiplier;
	j["dry_base"] = v.dry_base;
	j["dry_allowed_length"] = v.dry_allowed_length;
	j["dry_penalty_last_n"] = v.dry_penalty_last_n;

	j["dynatemp_range"] = v.dynatemp_range;
	j["dynatemp_exp"] = v.dynatemp_exp;

	j["mirostat"] = v.mirostat;
	j["mirostat_lr"] = v.mirostat_lr;
	j["mirostat_ent"] = v.mirostat_ent;

	j["grammar"] = v.grammar;
	j["json_schema"] = v.json_schema;
}

void from_json(const json &j, InferenceSettings &v)
{
	v.n_predict = j.value("n_predict", v.n_predict);
	v.samplers = j.value("samplers", v.samplers);

	v.seed = j.value("seed", v.seed);
	v.temperature = j.value("temperature", v.temperature);
	v.top_k = j.value("top_k", v.top_k);
	v.top_p = j.value("top_p", v.top_p);
	v.min_p = j.value("min_p", v.min_p);
	v.top_nsigma = j.value("top_nsigma", v.top_nsigma);
	v.typical_p = j.value("typical_p", v.typical_p);

	v.xtc_probability = j.value("xtc_probability", v.xtc_probability);
	v.xtc_threshold = j.value("xtc_threshold", v.xtc_threshold);

	v.repeat_last_n = j.value("repeat_last_n", v.repeat_last_n);
	v.repeat_penalty = j.value("repeat_penalty", v.repeat_penalty);
	v.presence_penalty = j.value("presence_penalty", v.presence_penalty);
	v.frequency_penalty = j.value("frequency_penalty", v.frequency_penalty);

	v.dry_multiplier = j.value("dry_multiplier", v.dry_multiplier);
	v.dry_base = j.value("dry_base", v.dry_base);
	v.dry_allowed_length = j.value("dry_allowed_length", v.dry_allowed_length);
	v.dry_penalty_last_n = j.value("dry_penalty_last_n", v.dry_penalty_last_n);

	v.dynatemp_range = j.value("dynatemp_range", v.dynatemp_range);
	v.dynatemp_exp = j.value("dynatemp_exp", v.dynatemp_exp);

	v.mirostat = j.value("mirostat", v.mirostat);
	v.mirostat_lr = j.value("mirostat_lr", v.mirostat_lr);
	v.mirostat_ent = j.value("mirostat_ent", v.mirostat_ent);

	v.grammar = j.value("grammar", v.grammar);
	v.json_schema = j.value("json_schema", v.json_schema);
}

// ============================================================================
// UISettings serialization
// ============================================================================

void to_json(json &j, const UISettings &v)
{
	j = json{ { "theme", v.theme },
			  { "default_tab", v.default_tab },
			  { "show_system_panel", v.show_system_panel },
			  { "refresh_rate_ms", v.refresh_rate_ms } };
}

void from_json(const json &j, UISettings &v)
{
	v.theme = j.value("theme", v.theme);
	v.default_tab = j.value("default_tab", v.default_tab);
	v.show_system_panel = j.value("show_system_panel", v.show_system_panel);
	v.refresh_rate_ms = j.value("refresh_rate_ms", v.refresh_rate_ms);
}

// ============================================================================
// TerminalSettings serialization
// ============================================================================

void to_json(json &j, const TerminalSettings &v)
{
	j = json{ { "default_shell", v.default_shell },
			  { "initial_command", v.initial_command },
			  { "working_directory", v.working_directory },
			  { "default_cols", v.default_cols },
			  { "default_rows", v.default_rows } };
}

void from_json(const json &j, TerminalSettings &v)
{
	v.default_shell = j.value("default_shell", v.default_shell);
	v.initial_command = j.value("initial_command", v.initial_command);
	v.working_directory = j.value("working_directory", v.working_directory);
	v.default_cols = j.value("default_cols", v.default_cols);
	v.default_rows = j.value("default_rows", v.default_rows);
}

// ============================================================================
// ModelPreset serialization
// ============================================================================

void to_json(json &j, const ModelPreset &v)
{
	j["name"] = v.name;
	j["model"] = v.model;
	j["inference"] = v.inference;
}

void from_json(const json &j, ModelPreset &v)
{
	v.name = j.value("name", v.name);
	v.model = j.value("model", v.model);
	if (j.contains("inference"))
		v.inference = j["inference"].get<InferenceSettings>();
}

// ============================================================================
// UserConfig serialization (main container)
// ============================================================================

void to_json(json &j, const UserConfig &v)
{
	j["server"] = v.server;
	j["load"] = v.load;
	j["inference"] = v.inference;
	j["ui"] = v.ui;
	j["terminal"] = v.terminal;
	j["presets"] = v.presets;
}

void from_json(const json &j, UserConfig &v)
{
	if (j.contains("server"))
		v.server = j["server"].get<ServerSettings>();
	if (j.contains("load"))
		v.load = j["load"].get<LoadSettings>();
	if (j.contains("inference"))
		v.inference = j["inference"].get<InferenceSettings>();
	if (j.contains("ui"))
		v.ui = j["ui"].get<UISettings>();
	if (j.contains("terminal"))
		v.terminal = j["terminal"].get<TerminalSettings>();
	if (j.contains("presets"))
		v.presets = j["presets"].get<std::vector<ModelPreset>>();
}

} // namespace Config
