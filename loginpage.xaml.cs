using System.Text;
using System.Text.Json;

namespace RayMinder.Pages;

public partial class LoginPage : ContentPage
{
    private readonly HttpClient _httpClient;

    public LoginPage(HttpClient httpClient)
    {
        InitializeComponent();
        _httpClient = httpClient;
    }

    private async void OnLoginClicked(object sender, EventArgs e)
    {
        var username = UsernameEntry.Text;
        var password = PasswordEntry.Text;

        var user = new
        {
            Username = username,
            PasswordHash = password // Hash if needed
        };

        var json = JsonSerializer.Serialize(user);
        var content = new StringContent(json, Encoding.UTF8, "application/json");

        try
        {
            //TODO: Update URL to use configuration setting
            //E.g., var url = $"{AppSettings.ApiBaseUrl}/User/login";
            // Then use: var response = await _httpClient.PostAsync(url, content);
            // ERROR HERE IS PROBABLY DUE TO THE URL BEING HARDCODED ? 
            var response = await _httpClient.PostAsync("http://localhost:44380/User/login", content);

            if (response.IsSuccessStatusCode)
            {
                await DisplayAlert("Success", "Login successful!", "OK");
                Application.Current.MainPage = new AppShell();


                //TODO: 
                // Navigate to the main application page
                // Create a session or store authentication token if needed - to follow best practices
                // E.g., var token = await response.Content.ReadAsStringAsync();

                //create landing page / main application page.. - here display dashboard monitoring and function to scan for devices etc.
            }
            else
            {
                var error = await response.Content.ReadAsStringAsync();
                await DisplayAlert("Error", error, "OK");
            }
        }
        catch (Exception ex)
        {
            await DisplayAlert("Error", ex.Message, "OK");
        }
    }

    private async void OnRegisterClicked(object sender, EventArgs e)
    {
        await Navigation.PushAsync(new RegistrationPage());
    }
}