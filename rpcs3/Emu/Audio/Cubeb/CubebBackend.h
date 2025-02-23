#pragma once

#include <memory>
#include "Utilities/mutex.h"
#include "util/atomic.hpp"
#include "Emu/Audio/AudioBackend.h"

#include "cubeb/cubeb.h"

class CubebBackend final : public AudioBackend
{
public:
	CubebBackend();
	~CubebBackend() override;

	CubebBackend(const CubebBackend&) = delete;
	CubebBackend& operator=(const CubebBackend&) = delete;

	std::string_view GetName() const override { return "Cubeb"sv; }

	bool Initialized() override;
	bool Operational() override;
	bool DefaultDeviceChanged() override;

	bool Open(std::string_view dev_id, AudioFreq freq, AudioSampleSize sample_size, AudioChannelCnt ch_cnt) override;
	void Close() override;

	f64 GetCallbackFrameLen() override;

	void Play() override;
	void Pause() override;

private:
	static constexpr f64 AUDIO_MIN_LATENCY = 512.0 / 48000; // 10ms

	cubeb* m_ctx = nullptr;
	cubeb_stream* m_stream = nullptr;
#ifdef _WIN32
	bool m_com_init_success = false;
#endif

	std::array<u8, sizeof(float) * static_cast<u32>(AudioChannelCnt::SURROUND_7_1)> m_last_sample{};
	atomic_t<u8> full_sample_size = 0;

	atomic_t<bool> m_reset_req = false;

	shared_mutex m_dev_sw_mutex{};
	std::string m_current_device{};
	bool m_default_dev_changed = false;

	bool m_dev_collection_cb_enabled = false;

	// Cubeb callbacks
	static long data_cb(cubeb_stream* stream, void* user_ptr, void const* input_buffer, void* output_buffer, long nframes);
	static void state_cb(cubeb_stream* stream, void* user_ptr, cubeb_state state);
	static void device_collection_changed_cb(cubeb* context, void* user_ptr);

	void CloseUnlocked();

	std::tuple<cubeb_devid, std::string /* dev_ident */, u32 /* ch_cnt */> GetDevice(std::string_view dev_id = "");
	std::tuple<cubeb_devid, std::string /* dev_ident */, u32 /* ch_cnt */> GetDefaultDeviceAlt(AudioFreq freq, AudioSampleSize sample_size, AudioChannelCnt ch_cnt);
};
