#include "esp_timer.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#include "hal_system.h"

#define MAX_TIMERS 16

typedef struct {
    void (*callback)(void* arg);
    void* arg;
    uint64_t period_us;
    volatile bool running;
    pthread_t thread_handle;
    bool thread_valid;
} esp_timer_impl_t;

static esp_timer_impl_t timer_registry[MAX_TIMERS];
static int timer_count = 0;
static pthread_mutex_t registry_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread function for periodic timer
static void* timer_thread_func(void* arg) {
    esp_timer_impl_t* timer = (esp_timer_impl_t*)arg;
    
    while (timer->running) {
        usleep((unsigned int)timer->period_us);
        if (timer->running && timer->callback) {
            timer->callback(timer->arg);
        }
    }
    return NULL;
}

esp_err_t esp_timer_create(const esp_timer_create_args_t *create_args, esp_timer_handle_t *out_handle) {
    if (!create_args || !out_handle || !create_args->callback) {
        return ESP_ERR_INVALID_ARG;
    }

    pthread_mutex_lock(&registry_mutex);
    
    if (timer_count >= MAX_TIMERS) {
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_ARG;  // No more space
    }

    esp_timer_impl_t* timer = &timer_registry[timer_count];
    memset(timer, 0, sizeof(*timer));
    timer->callback = create_args->callback;
    timer->arg = create_args->arg;
    timer->period_us = 0;
    timer->running = false;
    timer->thread_valid = false;

    int handle_id = timer_count++;
    *out_handle = (esp_timer_handle_t)(intptr_t)handle_id;

    pthread_mutex_unlock(&registry_mutex);
    return ESP_OK;
}

esp_err_t esp_timer_start_periodic(esp_timer_handle_t timer, uint64_t period_us) {
    int handle_id = (int)(intptr_t)timer;
    
    pthread_mutex_lock(&registry_mutex);
    
    if (handle_id < 0 || handle_id >= timer_count) {
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_ARG;
    }

    esp_timer_impl_t* t = &timer_registry[handle_id];
    if (t->running) {
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_STATE;
    }

    t->period_us = period_us;
    t->running = true;

    int ret = pthread_create(&t->thread_handle, NULL, timer_thread_func, t);
    if (ret != 0) {
        t->running = false;
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_ARG;
    }
    
    t->thread_valid = true;
    pthread_mutex_unlock(&registry_mutex);
    return ESP_OK;
}

esp_err_t esp_timer_stop(esp_timer_handle_t timer) {
    int handle_id = (int)(intptr_t)timer;
    
    pthread_mutex_lock(&registry_mutex);
    
    if (handle_id < 0 || handle_id >= timer_count) {
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_ARG;
    }

    esp_timer_impl_t* t = &timer_registry[handle_id];
    t->running = false;
    
    if (t->thread_valid) {
        pthread_mutex_unlock(&registry_mutex);
        pthread_join(t->thread_handle, NULL);
        pthread_mutex_lock(&registry_mutex);
        t->thread_valid = false;
    }
    
    pthread_mutex_unlock(&registry_mutex);
    return ESP_OK;
}

esp_err_t esp_timer_delete(esp_timer_handle_t timer) {
    int handle_id = (int)(intptr_t)timer;
    
    pthread_mutex_lock(&registry_mutex);
    
    if (handle_id < 0 || handle_id >= timer_count) {
        pthread_mutex_unlock(&registry_mutex);
        return ESP_ERR_INVALID_ARG;
    }

    esp_timer_impl_t* t = &timer_registry[handle_id];
    if (t->running) {
        pthread_mutex_unlock(&registry_mutex);
        esp_timer_stop(timer);
        pthread_mutex_lock(&registry_mutex);
    }
    
    memset(t, 0, sizeof(*t));
    pthread_mutex_unlock(&registry_mutex);
    return ESP_OK;
}

int64_t esp_timer_get_time(void) {
    return (int64_t)hal_system_uptime_ms() * 1000;  // Convert ms to us
}