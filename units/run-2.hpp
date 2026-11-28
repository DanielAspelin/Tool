std::string run(TOOL& tool, const std::vector<IR::Step>& steps) {
    std::map<std::string, std::string> values;
    std::size_t retained = 0;
    std::string output;
    const auto publish = [&](const IR::Step& step, std::string result) {
        if (result.size() > 262144 - retained) throw std::length_error("Script results exceed 256 KiB");
        retained += result.size();
        if (step.mode != IR::Mode::bind) {
            if (!output.empty()) output += '\n';
            output += step.name + ": " + result;
        }
        values.insert_or_assign(step.name, std::move(result));
    };
    const auto dispatch = [&tool](std::string command, Words args, std::uint64_t id) {
        const TOOL::InvocationContext context{id, std::chrono::steady_clock::now() + std::chrono::seconds(3)};
        for (;;) {
            auto status = TOOL::InvocationStatus::untracked;
            auto result = tool.invoke(command, args, &context, &status);
            if (status == TOOL::InvocationStatus::completed) return result;
            if (status != TOOL::InvocationStatus::rejected ||
                result != "Capacity unavailable; retry later" || context.expired()) throw std::runtime_error(result);
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }
    };
    for (std::size_t index = 0; index < steps.size();) {
        const auto& step = steps[index];
        auto args = resolve(step.parameters, values);
        if (step.mode == IR::Mode::bind) {
            publish(step, std::move(args.front()));
            ++index;
        } else if (step.mode == IR::Mode::serial) {
            publish(step, dispatch(step.command, std::move(args), index + 1));
            ++index;
        } else {
            const auto first = index;
            std::vector<std::future<std::string>> futures;
            while (index < steps.size() && steps[index].mode == IR::Mode::parallel) {
                auto parameters = index == first ? std::move(args) : resolve(steps[index].parameters, values);
                futures.push_back(std::async(std::launch::async, dispatch, steps[index].command, std::move(parameters), index + 1));
                ++index;
            }
            for (std::size_t item = 0; item < futures.size(); ++item) publish(steps[first + item], futures[item].get());
        }
    }
    return output;
}
