import subprocess
import unittest
import random


class TestAsyncServer(unittest.TestCase):

    @classmethod
    def setUpClass(cls) -> None:
        cls._srv_process = subprocess.Popen("../cmake/build_windows/Debug/coro_server.exe")

    @classmethod
    def tearDownClass(cls) -> None:
        cls._srv_process.terminate()


    def testOneClient(self):

        client_process = subprocess.Popen(["python", "../src_client/test_client.py"])

        self.assertEqual(client_process.wait(), 0)

    def testConcurrent(self):

        random.seed(42)

        client_processes = []

        for i in range(5):
            seed = random.randint(0, 1000)
            length = random.randint(10, 100)
            client_processes.append( subprocess.Popen(["python", "../src_client/test_client.py", "-s", str(seed), "-l", str(length)]) )

        for process in client_processes:
            self.assertEqual(process.wait(), 0)

if __name__ == '__main__':
    unittest.main()