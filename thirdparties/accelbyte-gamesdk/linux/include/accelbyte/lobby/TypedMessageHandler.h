// Copyright (c) 2023 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "MessageHandler.h"
#include "lobby_exports.h"

#include <accelbyte/common/Map.h>

// STL
#include <functional>
#include <iostream>

namespace accelbyte {
namespace lobby {

class TypedMessageHandlerBase : public MessageHandler {
public:
    TypedMessageHandlerBase() = default;

    virtual Set<String> handled_messages() const override { return handled_messages_; };

protected:
    Set<String> handled_messages_;
    Map<String, std::function<void(const Message&)>> handle_message_func_;
};

template<class L, class... R>
class TypedMessageHandler;

template<class TypedMessage>
class TypedMessageHandler<TypedMessage> : virtual public TypedMessageHandlerBase {
public:
    static_assert(std::is_base_of<Message, TypedMessage>::value, "TypedMessage must be derived from Message");

    TypedMessageHandler()
    {
        handled_messages_.insert(TypedMessage::message_id());
        handle_message_func_[TypedMessage::message_id()] =
            std::bind(&TypedMessageHandler<TypedMessage>::do_handle_message, this, std::placeholders::_1);
    }

    virtual void handle(const TypedMessage& message) = 0;

    void do_handle_message(const Message& message)
    {
        if (message.id() == TypedMessage::message_id()) {
            handle(static_cast<const TypedMessage&>(message));
            return;
        }
        std::cout << "unknown message" << std::endl;
    }

    void handle_message(const Message& message) override { do_handle_message(message); }
};

template<class TypedMessage, class... RemaningMessage>
class TypedMessageHandler
    : public TypedMessageHandler<TypedMessage>
    , public TypedMessageHandler<RemaningMessage...>
    , virtual public TypedMessageHandlerBase {
public:
    void handle_message(const Message& message) override
    {
        if (handle_message_func_.count(message.id()) != 0) {
            handle_message_func_[message.id()](message);
            return;
        }
        std::cout << "unknown message" << std::endl;
    }
};

} // namespace lobby
} // namespace accelbyte