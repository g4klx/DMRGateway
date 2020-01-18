This is the DMR Gateway which allows for the connection of up to six different DMR networks to one MMDVM system. One of the networks is defined as being an XLX reflector, while the other five may be any combination of DMR+, BrandMeister, TGIF, or local HBLink systems.

This software works by use of powerful rewriting rules which allow for changes in the slot, talk group, the type, and even the destination, of the messages. Without a rewrite rule, even if it does no actual rewriting, traffic will not be passed through from that defined network to the MMDVM and back again.

For example, the default configuration moves the announcements from BrandMeister for linking and unlinking to the same talk group slot as the reflectors themselves, a far more reasonable configuration than the default BrandMeister one.

The rewrite rules don’t apply to the XLX reflector, where only the slot and the talk group used may be changed. The controls i.e. private calls, for altering the reflector are fixed. In the case of the XLX reflectors the gateway will issue voice prompts to indicate the current reflector. These are available in a number of languages.

The MMDVM .ini file should have the IP address and port number of the client in the [DMR Network] settings.

This software builds on 32-bit and 64-bit Linux systems as well as on Windows using Visual Studio 2017 on x86 and x64.

This software is licenced under the GPL v2 and is intended for amateur and educational use only. Use of this software for commercial purposes is strictly forbidden.
