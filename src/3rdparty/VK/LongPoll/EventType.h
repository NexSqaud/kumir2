#pragma once

enum class EventType : int {
	ChangeMessageFlags = 1,
	SetMessageFlags = 2,
	ResetMessageFlags = 3,
	NewMessage = 4,
	EditMessage = 5,
	ReadAllIncomingMessageBeforeLocalId = 6,
	ReadAllOutgoingMessageBeforeLocalId = 7,
	FriendBecomeOnline = 8,
	FriendBecomeOffline = 9,
	ResetConversationFlags = 10,
	SwitchConversationFlags = 11,
	SetConversationFlags = 12,
	DeleteAllConversationMessage = 13,
	RestoreConversationMessage = 14,
	ChangeConversationSettings = 51,
	ChangeConversationInfo = 52,
	UserIsTypingNow = 61,
	UserTyping = 62,
	UsersTyping = 63,
	UsersRecording = 64,
	UserCalling = 70,
	SetCounter = 80,
	ChangeNotificationSettings = 114
};
