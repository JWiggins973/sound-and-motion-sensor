using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.EntityFrameworkCore;

// inhertits from  built in pagemodel
public class AlertLogModel : PageModel
{
    // once assigned cant be changed
    private readonly AlertDbContext _db;

    // list to hold db rows
    public List<AlertEvent> Alerts { get; set; } = new();

    // ASP.NET Core auto-provides db connection here, stores it
    public AlertLogModel(AlertDbContext db)
    {
        _db = db;
    }

    // updates page on load and relaods
    public async Task OnGetAsync()
    {
        Alerts = await _db.Alerts.ToListAsync();
    }
}