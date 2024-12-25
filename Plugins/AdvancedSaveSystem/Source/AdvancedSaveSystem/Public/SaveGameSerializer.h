#pragma once



class UAdvancedSaveGameSubsystem;

class FSaveGameSerializer : TSharedFromThis<FSaveGameSerializer>
{
	public:
		virtual ~FSaveGameSerializer() = default;
};
