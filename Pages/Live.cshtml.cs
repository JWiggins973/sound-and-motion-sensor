using Microsoft.AspNetCore.Mvc.RazorPages;

public class LiveModel : PageModel
{
    // ref to livestatus store
    private readonly LiveStatusStore _store;

    // LiveStatusDtos to be used in UI
    public LiveStatusDtos LiveStatus { get; private set; } = new();

    // constructor to assign the LiveStatusStore instance
    public LiveModel(LiveStatusStore store)
    {
        _store = store;
    }

    // Runs on every GET request
     public void OnGet()
    {
        LiveStatus = _store.GetLiveStatusDtos();
    }

}