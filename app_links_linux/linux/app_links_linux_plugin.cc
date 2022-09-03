#include "include/app_links_linux/app_links_linux_plugin.h"

#include <flutter_linux/flutter_linux.h>
#include <gtk/gtk.h>
#include <sys/utsname.h>

#include <cstring>

// Constants
const char kMethodChannel[] = "com.llfbandit.app_links/messages";
const char kEventChannel[] = "com.llfbandit.app_links/events";
const char kGetInitialAppLinkMethod[] = "getInitialAppLink";
const char kGetLatestAppLinkMethod[] = "getLatestAppLink";
#define APP_LINKS_LINUX_PLUGIN(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), app_links_linux_plugin_get_type(), \
                              AppLinksLinuxPlugin))

struct _AppLinksLinuxPlugin {
  GObject parent_instance;

  FlPluginRegistrar* registrar;

  // Connection to Flutter engine.
  FlMethodChannel* channel;

  gchar* initial_link_;
  gchar* latest_link_;
};

G_DEFINE_TYPE(AppLinksLinuxPlugin, app_links_linux_plugin, g_object_get_type())


static FlMethodResponse* get_initial_app_link(FlUrlLauncherPlugin* self, FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_string(self->initial_link_);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

static FlMethodResponse* get_latest_app_link(FlUrlLauncherPlugin* self, FlValue* args) {
  g_autoptr(FlValue) result = fl_value_new_string(self->latest_link_);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

// Called when a method call is received from Flutter.
static void app_links_linux_plugin_handle_method_call(
    AppLinksLinuxPlugin* self,
    FlMethodCall* method_call) {
  g_autoptr(FlMethodResponse) response = nullptr;

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  if (strcmp(method, kGetInitialAppLinkMethod) == 0) 
    response = get_initial_app_link(self, args);
  else if (strcmp(method, kGetLatestAppLinkMethod) == 0)
    response = get_latest_app_link(self, args)
  else 
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error))
    g_warning("Failed to send method call response: %s", error->message);
}

static void method_call_cb(FlMethodChannel* channel, FlMethodCall* method_call,
                           gpointer user_data) {
  AppLinksLinuxPlugin* plugin = APP_LINKS_LINUX_PLUGIN(user_data);
  app_links_linux_plugin_handle_method_call(plugin, method_call);
}

static void app_links_linux_plugin_dispose(GObject* object) {
  AppLinksLinuxPlugin* plugin = APP_LINKS_LINUX_PLUGIN(object);

  g_clear_object(&self->registrar);
  g_clear_object(&self->channel);

  g_free (&self->initial_link_);
  g_free (&self->latest_link_);
  G_OBJECT_CLASS(app_links_linux_plugin_parent_class)->dispose(object);
}

static void app_links_linux_plugin_class_init(AppLinksLinuxPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = app_links_linux_plugin_dispose;
}

static void app_links_linux_plugin_init(AppLinksLinuxPlugin* self) {}

AppLinksLinuxPlugin* app_links_linux_plugin_new(FlPluginRegistrar* registrar) {
  AppLinksLinuxPlugin* self = APP_LINKS_LINUX_PLUGIN(
      g_object_new(app_links_linux_plugin_get_type(), nullptr));

  self->registrar = FL_PLUGIN_REGISTRAR(g_object_ref(registrar));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->channel =
      fl_method_channel_new(fl_plugin_registrar_get_messenger(registrar),
                            kMethodChannel,
                            FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(channel, method_call_cb,
                                            g_object_ref(plugin),
                                            g_object_unref);
}

void app_links_linux_plugin_register_with_registrar(FlPluginRegistrar* registrar) {
  AppLinksLinuxPlugin* plugin = app_links_linux_plugin_new(registrar);

  g_object_unref(plugin);
}
