from locust import HttpUser, task, between


class StaticWebUser(HttpUser):
    # Затримка між запитами (імітує реального користувача)
    wait_time = between(1, 3)

    @task(3)
    def load_index(self):
        """Завантаження головної сторінки (найчастіший запит)"""
        self.client.get("/index.html")

    @task(2)
    def load_page2(self):
        """Завантаження другої сторінки"""
        self.client.get("/page2.html")

    @task(1)
    def load_root(self):
        """Запит кореневого шляху — сервер має повернути index.html"""
        self.client.get("/")

    @task(1)
    def load_404(self):
        """Запит неіснуючого файлу — очікуємо 404"""
        # rescue=False щоб locust не вважав 404 помилкою тесту
        with self.client.get("/nonexistent.html", catch_response=True) as response:
            if response.status_code == 404:
                response.success()
            else:
                response.failure(f"Expected 404, got {response.status_code}")
