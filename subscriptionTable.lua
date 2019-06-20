-- 
-- A table entry has the following format:
--
-- key: <topic:string>
-- value: table { 
--          key: "filename", value: <filename:string>,
--          key: "timer", value: <seconds:int>,        (optional)
--        }
--
subscriptionTable = {
    ["temperature"] = {
        ["filename"] = "temperature.lua",
        ["timer"] = 10,
    },

    ["emails"] = {
        ["filename"] = "emails.lua",
    },
}
