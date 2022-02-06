local M = {}

M.DISPLAY_WIDTH = tonumber(sys.get_config("display.width"))
M.DISPLAY_HEIGHT = tonumber(sys.get_config("display.height"))

M.locked = false

M.cursor_x = M.DISPLAY_WIDTH / 2
M.cursor_y = M.DISPLAY_HEIGHT / 2
M.cursor_visible = true

function M.transform_input(action_id, action)
    action.x = M.cursor_x
    action.y = M.cursor_y

    if not M.locked or not M.cursor_visible then
        action_id = nil
    end

    return action_id, action
end

function M.lock()
    if not M.locked then
        if html5 then
            cursor_lock_ext.html5_request_cursor_lock()
        else
            M.locked = true
            cursor_lock_ext.glfw_mouse_lock()
        end
        return true
    end
    return false
end

function M.unlock()
    if html5 then
        if M.locked then
            cursor_lock_ext.html5_exit_cursor_lock()
            return true
        end
    else
        local was_locked = M.locked
        M.locked = false
        cursor_lock_ext.glfw_mouse_unlock()
        return was_locked
    end
end

return M