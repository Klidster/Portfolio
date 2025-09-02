using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class GunController : MonoBehaviour
{
    [Header("Inventory")]
    public Inventory inventory;
    public InventoryItem gunItem;

    [Header("UI")]
    public Image reticle;

    [Header("Shooting")]
    public GameObject bulletDecalPrefab;
    public float fireRate = 0.25f;
    public AudioClip destroySfx;
    public AudioClip shootSfx;

    [Header("Disable While Aiming")]
    public ObjectInteractor objectInteractor;

    bool _active = false;
    float _nextFire = 0f;

    void Start()
    {
        if (reticle != null) reticle.enabled = false;
        inventory.ItemUsed += OnItemUsed;
    }

    void OnDestroy()
    {
        inventory.ItemUsed -= OnItemUsed;
    }

    void OnItemUsed(InventoryItem item)
    {
        if (item != gunItem) return;

        _active = !_active;
        if (reticle != null) reticle.enabled = _active;

        if (objectInteractor != null)
            objectInteractor.enabled = !_active;
    }

    void Update()
    {
        if (!_active) return;

        if (Input.GetButton("Fire1") && Time.time >= _nextFire)
        {
            _nextFire = Time.time + fireRate;
            Fire();
            AudioSource.PlayClipAtPoint(shootSfx, Camera.main.transform.position, 0.6f);
        }
    }

    void Fire()
    {
        if (Physics.Raycast(
                Camera.main.transform.position,
                Camera.main.transform.forward,
                out var hit,
                Mathf.Infinity))
        {
            if (hit.collider.CompareTag("Target"))
            {
                Destroy(hit.collider.gameObject);
                AudioSource.PlayClipAtPoint(destroySfx, Camera.main.transform.position, 0.4f);
                return;
            }

            var decal = Instantiate(
                bulletDecalPrefab,
                hit.point + hit.normal * 0.001f,
                Quaternion.FromToRotation(-Vector3.forward, hit.normal)
            );
            Destroy(decal, 5f);
        }
    }
}

