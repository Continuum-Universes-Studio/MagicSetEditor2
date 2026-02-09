#include <gui_core/startup_flow.hpp>

#include <cli/cli_main.hpp>
#include <cli/text_io_handler.hpp>
#include <data/installer.hpp>
#include <data/format/formats.hpp>
#include <data/set.hpp>
#include <data/settings.hpp>

#include <filesystem>

ScriptValueP export_set(SetP const& set, vector<CardP> const& cards, ExportTemplateP const& exp, String const& outname);

namespace gui_core {
namespace {
String file_extension(const String& input) {
  String file = input;
  while (!file.empty() && (file.Last() == _('/') || file.Last() == _('\\'))) {
    file.RemoveLast();
  }
  size_t dot = file.find_last_of(_('.'));
  if (dot == String::npos) {
    return _("");
  }
  String ext = file.substr(dot + 1);
  ext.MakeLower();
  return ext;
}

vector<String> startup_args(int argc, char** argv) {
  vector<String> args;
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
    if (args.back() == _("--color")) {
      args.pop_back();
    }
  }
  return args;
}

void print_help(const char* executable_name) {
  cli << _("Magic Set Editor\n\n");
  cli << _("Usage: ") << BRIGHT << executable_name << NORMAL << _(" [") << PARAM << _("OPTIONS") << NORMAL << _("]");
  cli << _("\n\n  no options");
  cli << _("\n         \tStart the MSE user interface showing the welcome window.");
  cli << _("\n\n  ") << BRIGHT << _("-?") << NORMAL << _(", ") << BRIGHT << _("--help") << NORMAL;
  cli << _("\n         \tShows this help screen.");
  cli << _("\n\n  ") << BRIGHT << _("-v") << NORMAL << _(", ") << BRIGHT << _("--version") << NORMAL;
  cli << _("\n         \tShow version information.");
  cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-set") << NORMAL << _(", ") << PARAM << _("FILE") << FILE_EXT << _(".set") << NORMAL << _(", ") << PARAM << _("FILE") << FILE_EXT << _(".mse") << NORMAL;
  cli << _("\n         \tLoad the set file in the MSE user interface.");
  cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-symbol") << NORMAL;
  cli << _("\n         \tLoad the symbol into the MSE symbol editor.");
  cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-installer") << NORMAL << _(" [") << BRIGHT << _("--local") << NORMAL << _("]");
  cli << _("\n         \tInstall the packages from the installer.");
  cli << _("\n         \tIf the ") << BRIGHT << _("--local") << NORMAL << _(" flag is passed, install packages for this user only.");
  cli << _("\n\n  ") << PARAM << _("FILE") << FILE_EXT << _(".mse-script") << NORMAL;
  cli << _("\n         \tRun a script file.");
  cli << _("\n\n  ") << BRIGHT << _("--symbol-editor") << NORMAL;
  cli << _("\n         \tShow the symbol editor instead of the welcome window.");
  cli << _("\n\n  ") << BRIGHT << _("--create-installer") << NORMAL << _(" [") << PARAM << _("OUTFILE") << FILE_EXT << _(".mse-installer") << NORMAL << _("] [") << PARAM << _("PACKAGE") << NORMAL << _(" [") << PARAM << _("PACKAGE") << NORMAL << _(" ...]]");
  cli << _("\n         \tCreate an instaler, containing the listed packages.");
  cli << _("\n         \tIf no output filename is specified, the name of the first package is used.");
  cli << _("\n\n  ") << BRIGHT << _("--export") << NORMAL << PARAM << _(" TEMPLATE SETFILE ") << NORMAL << _(" [") << PARAM << _("OUTFILE") << NORMAL << _("]");
  cli << _("\n         \tExport a set using an export template.");
  cli << _("\n         \tIf no output filename is specified, the result is written to stdout.");
  cli << _("\n\n  ") << BRIGHT << _("--export-images") << NORMAL << PARAM << _(" FILE") << NORMAL << _(" [") << PARAM << _("IMAGE") << NORMAL << _("]");
  cli << _("\n         \tExport the cards in a set to image files,");
  cli << _("\n         \tIMAGE is the same format as for 'export all card images'.");
  cli << _("\n\n  ") << BRIGHT << _("--cli") << NORMAL << _(" [") << PARAM << _("FILE") << NORMAL << _("] [") << BRIGHT << _("--quiet") << NORMAL << _("] [") << BRIGHT << _("--raw") << NORMAL << _("]");
  cli << _("\n         \tStart the command line interface for performing commands on the set file.");
  cli << _("\n         \tUse ") << BRIGHT << _("-q") << NORMAL << _(" or ") << BRIGHT << _("--quiet") << NORMAL << _(" to supress the startup banner and prompts.");
  cli << _("\n         \tUse ") << BRIGHT << _("-raw") << NORMAL << _(" for raw output mode.");
  cli << ENDL;
  cli.flush();
}
} // namespace

StartupRequest parse_startup_request(int argc, char** argv, InstallType default_install_type) {
  StartupRequest request;
  request.install_type = default_install_type;
  request.args = startup_args(argc, argv);
  if (request.args.empty()) {
    return request;
  }

  request.argument = request.args[0];
  const String extension = file_extension(request.argument);
  if (extension == _("mse-symbol")) {
    request.route = StartupRoute::OpenSymbolFile;
  } else if (extension == _("mse-set") || extension == _("mse") || extension == _("set")) {
    request.route = StartupRoute::OpenSetFile;
  } else if (extension == _("mse-installer")) {
    if (request.args.size() > 1 && starts_with(request.args[1], _("--"))) {
      parse_enum(String(request.args[1]).substr(2), request.install_type);
    }
    request.route = StartupRoute::OpenInstaller;
  } else if (extension == _("mse-script")) {
    request.route = StartupRoute::RunScript;
  } else if (request.argument == _("--symbol-editor")) {
    request.route = StartupRoute::OpenSymbolEditor;
  } else if (request.argument == _("--create-installer")) {
    request.route = StartupRoute::CreateInstaller;
  } else if (request.argument == _("--help") || request.argument == _("-?")) {
    request.route = StartupRoute::ShowHelp;
  } else if (request.argument == _("--version") || request.argument == _("-v") || request.argument == _("-V")) {
    request.route = StartupRoute::ShowVersion;
  } else if (request.argument == _("--cli")) {
    request.route = StartupRoute::RunCli;
  } else if (request.argument == _("--export-images")) {
    request.route = StartupRoute::ExportImages;
  } else if (request.argument == _("--export")) {
    request.route = StartupRoute::ExportSet;
  } else {
    request.route = StartupRoute::InvalidArgument;
  }
  return request;
}

int run_startup_request(const StartupRequest& request, StartupPresenter& presenter, const char* executable_name) {
  switch (request.route) {
    case StartupRoute::ShowWelcome:
      return presenter.showWelcomeWindow();
    case StartupRoute::OpenSymbolFile:
      return presenter.showSymbolEditor(request.argument);
    case StartupRoute::OpenSetFile:
      return presenter.showSetEditor(request.argument);
    case StartupRoute::OpenInstaller:
      return presenter.showInstaller(request.argument, request.install_type);
    case StartupRoute::RunScript:
      if (!run_script_file(request.argument)) return EXIT_FAILURE;
      return cli.shown_errors() ? EXIT_FAILURE : EXIT_SUCCESS;
    case StartupRoute::OpenSymbolEditor:
      return presenter.showSymbolEditor(_(""));
    case StartupRoute::CreateInstaller: {
      Installer inst;
      FOR_EACH(arg, request.args) {
        if (!starts_with(arg, _("--"))) {
          inst.addPackage(arg);
        }
      }
      if (inst.prefered_filename.empty()) {
        throw Error(_("Specify packages to include in installer"));
      }
      inst.saveAs(inst.prefered_filename, false);
      return EXIT_SUCCESS;
    }
    case StartupRoute::ShowHelp:
      print_help(executable_name);
      return EXIT_SUCCESS;
    case StartupRoute::ShowVersion:
      cli << _("Magic Set Editor\n");
      cli << _("Version ") << app_version.toString() << version_suffix << ENDL;
      cli.flush();
      return EXIT_SUCCESS;
    case StartupRoute::RunCli: {
      SetP set;
      bool quiet = false;
      for (size_t i = 1; i < request.args.size(); ++i) {
        const String& arg = request.args[i];
        const String ext = file_extension(arg);
        if (ext == _("mse-set") || ext == _("mse") || ext == _("set")) {
          set = import_set(arg);
        } else if (arg == _("-q") || arg == _("--quiet")) {
          quiet = true;
        } else if (arg == _("-r") || arg == _("--raw")) {
          quiet = true;
          cli.enableRaw();
        }
      }
      CLISetInterface cli_interface(set, quiet);
      return EXIT_SUCCESS;
    }
    case StartupRoute::ExportImages: {
      if (request.args.size() < 2) {
        handle_error(Error(_("No input file specified for --export")));
        return EXIT_FAILURE;
      }
      SetP set = import_set(request.args[1]);
      String out = request.args.size() >= 3 && !starts_with(request.args[2], _("--"))
        ? request.args[2]
        : settings.gameSettingsFor(*set->game).images_export_filename;
      String path = _(".");
      size_t pos = out.find_last_of(_("/\\"));
      if (pos != String::npos) {
        path = out.substr(0, pos);
        std::filesystem::create_directories(path.ToStdString());
        path += _("/x");
        out = out.substr(pos + 1);
      }
      export_images(set, set->cards, path, out, CONFLICT_NUMBER_OVERWRITE);
      return EXIT_SUCCESS;
    }
    case StartupRoute::ExportSet: {
      if (request.args.size() < 2) throw Error(_("No export template specified for --export"));
      if (request.args.size() < 3) throw Error(_("No input set file specified for --export"));
      ExportTemplateP exp = ExportTemplate::byName(request.args[1]);
      SetP set = import_set(request.args[2]);
      String out = request.args.size() >= 4 ? request.args[3] : _("");
      ScriptValueP result = export_set(set, set->cards, exp, out);
      if (out.empty()) {
        cli << result->toString();
      }
      return EXIT_SUCCESS;
    }
    case StartupRoute::InvalidArgument:
      handle_error(_("Invalid command line argument:\n") + request.argument);
      return presenter.showWelcomeWindow();
  }
  return EXIT_FAILURE;
}

} // namespace gui_core
