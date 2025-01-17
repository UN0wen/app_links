#include "app_links_plugin.h"

using namespace flutter;

namespace applinks_plugin {
	// static
	void AppLinksWindowsPlugin::RegisterWithRegistrar(flutter::PluginRegistrarWindows* registrar) {
		auto plugin = std::make_unique<AppLinksWindowsPlugin>(registrar);

		auto methodChannel = std::make_unique<FlMethodChannel>(
			registrar->messenger(), "com.llfbandit.app_links/messages",
			&flutter::StandardMethodCodec::GetInstance());

		methodChannel->SetMethodCallHandler(
			[plugin_pointer = plugin.get()](const auto& call, auto result) {
				plugin_pointer->HandleMethodCall(call, std::move(result));
			}
		);

		auto eventChannel = std::make_unique<FlEventChannel>(
			registrar->messenger(), "com.llfbandit.app_links/events",
			&flutter::StandardMethodCodec::GetInstance());

		auto eventHandler = std::make_unique<StreamHandlerFunctions<EncodableValue>>(
			[plugin_pointer = plugin.get()]
			(const EncodableValue* arguments, std::unique_ptr<flutter::EventSink<flutter::EncodableValue>>&& events)->std::unique_ptr<flutter::StreamHandlerError<flutter::EncodableValue>> {
				 return plugin_pointer->OnListen(arguments, std::move(events));
			},
			[plugin_pointer = plugin.get()]
			(const EncodableValue* arguments)->std::unique_ptr<FlStreamHandlerError> {
				return plugin_pointer->OnCancel(arguments);
			}
		);

		eventChannel->SetStreamHandler(std::move(eventHandler));

		registrar->AddPlugin(std::move(plugin));
	}

	AppLinksWindowsPlugin::AppLinksWindowsPlugin(PluginRegistrarWindows* registrar)
		: registrar_(registrar) {

		window_proc_id_ = registrar->RegisterTopLevelWindowProcDelegate(
			[this](HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
				return HandleWindowProc(hwnd, message, wparam, lparam);
			});
	}

	AppLinksWindowsPlugin::~AppLinksWindowsPlugin() {
		registrar_->UnregisterTopLevelWindowProcDelegate(window_proc_id_);
	}

	void AppLinksWindowsPlugin::HandleMethodCall(
		const flutter::MethodCall<flutter::EncodableValue>& method_call,
		std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
		if (method_call.method_name().compare("getInitialAppLink") == 0) {
			result->Success(flutter::EncodableValue(initialLink_.value_or("")));
		}
		else if (method_call.method_name().compare("getLatestAppLink") == 0) {
			result->Success(flutter::EncodableValue(latestLink_.value_or("")));
		}
		else {
			result->NotImplemented();
		}
	}

	std::optional<LRESULT> AppLinksWindowsPlugin::HandleWindowProc(
		HWND hwnd,
		UINT message,
		WPARAM wparam,
		LPARAM lparam) {

		if (message == WM_COPYDATA) {
			COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lparam;

			if (cds->dwData == APPLINK_MSG_ID) {
				std::string link((char*)(cds->lpData));

				if (!initialLink_.has_value()) {
					initialLink_ = link;
				}

				latestLink_ = link;

				if (eventSink_) {
					eventSink_->Success(latestLink_.value());
				}
			}
		}

		return std::nullopt;
	}

	std::unique_ptr<FlStreamHandlerError> AppLinksWindowsPlugin::OnListen(
		const flutter::EncodableValue* arguments,
		std::unique_ptr<FlEventSink>&& events) {

		eventSink_ = std::move(events);
		return nullptr;
	}

	std::unique_ptr<FlStreamHandlerError> AppLinksWindowsPlugin::OnCancel(
		const flutter::EncodableValue* arguments) {
		eventSink_ = nullptr;
		return nullptr;
	}

}  // namespace
