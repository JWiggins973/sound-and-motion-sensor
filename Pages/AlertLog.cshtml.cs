using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.Mvc.RazorPages;
using Microsoft.EntityFrameworkCore;

public class AlertLogModel : PageModel
{
    // Refrence to the db context assigned once and never assigned 
    private readonly AlertDbContext _db;

    // AlertsEvent list to store events drom db context to be used in UI
    public List<AlertEvent> Alerts { get; set; } = new();

    // Sets starting page number and allos to be fetched by http request
    [BindProperty(SupportsGet = true)] public int PageNumber { get; set; } = 1;

    // Allows filter to be fetched by http request and null when no filter applied
    [BindProperty(SupportsGet = true)] public string? Filter {get; set;}

    // Helps determine if there's a next page based on total rows
    public bool HasNextPage { get; set; }

    private const int PageSize = 20;

    // stores the AlertDbContext instance 
    public AlertLogModel(AlertDbContext db)
    {
        _db = db;
    }

    // Runs in every GET request  
    public async Task OnGetAsync()
    {
        var query = _db.Alerts.AsQueryable();
        
        // update on prebuilt querys clicks
        query = Filter switch
        {
            "today" => query.Where(a =>a.Timestamp.Date == DateTime.Today),
            "door" => query.Where(a => a.Message == "DOOR OPEN"),
            "loud" => query.Where(a => a.Message == "LOUD NOISE IN AREA"),
            _ => query
        };

        // Sort each page by date descending 
        query = query.OrderByDescending(a => a.Timestamp);

        // only show accurate page count based on rows
        int totalCount = await query.CountAsync();
        HasNextPage = PageSize * PageNumber < totalCount;

        Alerts = await query
            .Skip(PageSize * (PageNumber - 1))
            .Take(PageSize)
            .ToListAsync();
    }
}