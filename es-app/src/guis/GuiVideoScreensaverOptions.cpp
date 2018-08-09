#include "guis/GuiVideoScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Settings.h"

GuiVideoScreensaverOptions::GuiVideoScreensaverOptions(Window* window, const char* title) : GuiScreensaverOptions(window, title)
{
	// timeout to swap videos
	auto swap = std::make_shared<SliderComponent>(mWindow, 10.f, 1000.f, 1.f, "s");
	swap->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout") / (1000)));
	addWithLabel("TROCAR VÍDEO APÓS (SEGS)", swap);
	addSaveFunc([swap] {
		int playNextTimeout = (int)Math::round(swap->getValue()) * (1000);
		Settings::getInstance()->setInt("ScreenSaverSwapVideoTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

#ifdef _RPI_
	auto ss_omx = std::make_shared<SwitchComponent>(mWindow);
	ss_omx->setState(Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	addWithLabel("USAR OMX PLAYER NA PROTEÇÃO DE TELA", ss_omx);
	addSaveFunc([ss_omx, this] { Settings::getInstance()->setBool("ScreenSaverOmxPlayer", ss_omx->getState()); });
#endif

	// Render Video Game Name as subtitles
	auto ss_info = std::make_shared< OptionListComponent<std::string> >(mWindow, "EXIBIR INFORMAÇÃO DO JOGO", false);
	std::vector<std::string> info_type;
	info_type.push_back("always");
	info_type.push_back("start & end");
	info_type.push_back("never");
	for(auto it = info_type.cbegin(); it != info_type.cend(); it++)
		ss_info->add(*it, *it, Settings::getInstance()->getString("ScreenSaverGameInfo") == *it);
	addWithLabel("EXIBIR INFO. DO JOGO NA PROTEÇÃO DE TELA", ss_info);
	addSaveFunc([ss_info, this] { Settings::getInstance()->setString("ScreenSaverGameInfo", ss_info->getSelected()); });

#ifndef _RPI_
	auto captions_compatibility = std::make_shared<SwitchComponent>(mWindow);
	captions_compatibility->setState(Settings::getInstance()->getBool("CaptionsCompatibility"));
	addWithLabel("USAR BAIXA RESOLUÇÃO COMPATÍVEL PARA LEGENDAS", captions_compatibility);
	addSaveFunc([captions_compatibility] { Settings::getInstance()->setBool("CaptionsCompatibility", captions_compatibility->getState()); });
#endif

	auto stretch_screensaver = std::make_shared<SwitchComponent>(mWindow);
	stretch_screensaver->setState(Settings::getInstance()->getBool("StretchVideoOnScreenSaver"));
	addWithLabel("ESTICAR VÍDEO NA PROTEÇÃO DE TELA", stretch_screensaver);
	addSaveFunc([stretch_screensaver] { Settings::getInstance()->setBool("StretchVideoOnScreenSaver", stretch_screensaver->getState()); });
}

GuiVideoScreensaverOptions::~GuiVideoScreensaverOptions()
{
}

void GuiVideoScreensaverOptions::save()
{
#ifdef _RPI_
	bool startingStatusNotRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") == "never" || !Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
#endif
	GuiScreensaverOptions::save();

#ifdef _RPI_
	bool endStatusRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") != "never" && Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	if (startingStatusNotRisky && endStatusRisky) {
		// if before it wasn't risky but now there's a risk of problems, show warning
		mWindow->pushGui(new GuiMsgBox(mWindow,
		"USAR O OMX PLAYER E EXIBIR INFORMAÇÕES DO JOGO PODE FAZER COM QUE O VÍDEO FIQUE PISCANDO EM ALGUNS MODOS DE TV. SE ISSO ACONTECER, CONSIDERE:\N\N• DESATIVAR A OPÇÃO \"EXIBIR INFORMAÇÃO DO JOGO\";\n• DESATIVAR O \"Overscan\" NO MENU DE CONFIGURAÇÃO DO PI:\nRetroPie > Raspi-Config > OPÇÕES AVANÇADAS > Overscan > \"NÃO\".\n• DESATIVAR O USO DO OMX PLAYER PARA A PROTEÇÃO DE TELA.",
			"ENTENDI!", [] { return; }));
	}
#endif
}
