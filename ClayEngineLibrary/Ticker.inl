class App;

namespace ClayEngine
{
    struct TickThreadFunctor
    {
        void operator()(std::future<void> future, App* app);
    };

    class TickThread
    {
        std::thread t;
        std::promise<void> p{};

    public:
        TickThread(App* app);
        ~TickThread();
    };

    using TickThreadPtr = std::unique_ptr<ClayEngine::TickThread>;
}
