namespace RayMinder.Api.Model
{
    public class User
    {
        //TODO: Remove constant default credentials in production
        public const string DefaultUsername = "admin";
        public static readonly string DefaultPasswordHash = "your_hashed_password_here";

        public string Username { get; set; } = string.Empty;
        public string PasswordHash { get; set; } = string.Empty;
    }
}
