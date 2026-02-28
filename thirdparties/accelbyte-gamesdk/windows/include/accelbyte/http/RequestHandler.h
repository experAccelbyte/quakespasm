// Copyright (c) 2024 AccelByte Inc. All Rights Reserved.
// This is licensed software from AccelByte Inc, for limitations
// and restrictions contact your company contract manager.
#pragma once

#include "accelbyte/http/http_exports.h"
#include "accelbyte/http/Task.h"
#include "accelbyte/http/RequestExecutorFactory.h"
#include "accelbyte/http/RequestSchedulerFactory.h"
#include "accelbyte/memory/memory.h"



namespace accelbyte {
namespace http {


/**
* @brief interfaces to an http request handler.
**/
class AB_CNL_HTTP_EXPORT RequestHandler {
public:
    /**
    * @brief default detructor.
    **/
    virtual ~RequestHandler() = default;


    /**
    * @brief execute the request and call the callback that are in the task
    * @param[in] request the request to be sent
    * @param[in] executor_factory the executor factory to use to send this request 
    *                             if none is set the one that has been set by
                                  request_executor_factory::set_request_executor_factory()
                                  will be used
    * @param[in] scheduler_factory the scheduler factory to use to send this request 
    *                              if none is set the one that has been set by
                                   request_scheduler_factory::set_request_scheduler_factory()
                                   will be used
    **/
    virtual void execute_task(
        const Task& request, 
        memory::SharedPtr<const RequestExecutorFactory> executor_factory =
            memory::SharedPtr<const RequestExecutorFactory>(),
        memory::SharedPtr<const RequestSchedulerFactory> scheduler_factory =
            memory::SharedPtr<const RequestSchedulerFactory>()) const = 0;
};

}
}